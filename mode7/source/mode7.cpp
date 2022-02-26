// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>

// Engine code
#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Keypad.h>
#include <linearMath.h>
#include <Text.h>
#include <Timer.h>
#include <gfx/palette.h>
#include <gfx/sprite.h>
#include <gfx/tile.h>
#include <tools/frameCounter.h>

// Demo code
#include <demo.h>
#include <Camera.h>

using namespace math;

// Global Camera State
Vec3p8 gCamPos;
intp8 gCosf = 1_p8;
intp8 gSinf = 0_p8;

TextSystem text;

void initBackground()
{
	// Background clear color (used for blending too)
	gfx::BackgroundPalette::color(0).raw = BasicColor::SkyBlue.raw;
	// Prepare the background tile map
	uint32_t numColors = 4; // 2 in tile colors + 2 border colors
	auto paletteStart = gfx::BackgroundPalette::Allocator::alloc(2);
	gfx::BackgroundPalette::color(paletteStart + 0).raw = BasicColor::White.raw;
	gfx::BackgroundPalette::color(paletteStart + 1).raw = BasicColor::Red.raw;

	// Config BG2
	// Use charblock 0 for the tiles
	// Use the first screen block after charblock 0 (i.e. screenblock 8)
	// 128*128 map size
	IO::BG2CNT::Get().value =
		(1<<7) | // 16 bit color, technically not necessary as Affine backgrounds are always 16bit colors
		(8<<8) | // screenblock 8
		(3<<0xe); // size 1024*1024

	// Fill in a couple tiles in video memory
	auto& tileBank = gfx::TileBank::GetBank(0);
	auto tileStart = tileBank.allocDTiles(2);
    auto& tile0 = tileBank.GetDTile(tileStart + 0);
	tile0.fill(1); // White
	auto& tile1 = tileBank.GetDTile(tileStart + 1);
	tile1.fill(2); // Red

	// Fill in map data
	// Affine maps use 8 bit indices
	auto* mapMem = reinterpret_cast<volatile uint16_t*>(VideoMemAddress+0x4000);
	for(int32_t y = 0; y < 128; ++y)
	{
		for(int32_t x = 0; x < 64; ++x)
		{
			mapMem[y*64+x] = (y&1) ? 1 : (1<<8);
		}
	}
}

struct RasteredObj
{
	static constexpr auto ambientColor = BasicColor::Blue;
	static constexpr auto litColor = BasicColor::White;

	RasteredObj(Vec3p8 startPos)
		:m_pos(startPos)
	{
		// Allocate a palette of just two colors to do dithering with
		m_paletteStart = gfx::SpritePalette::Allocator::alloc(5);

		// Init palette
		gfx::SpritePalette::color(m_paletteStart+0).raw = Color(0.2f,0.2f,0.2f).raw; // No direct light -> ambient color
		gfx::SpritePalette::color(m_paletteStart+1).raw = Color(0.4f,0.4f,0.4f).raw;
		gfx::SpritePalette::color(m_paletteStart+2).raw = Color(0.6f,0.6f,0.6f).raw;
		gfx::SpritePalette::color(m_paletteStart+3).raw = Color(0.8f,0.8f,0.8f).raw;
		gfx::SpritePalette::color(m_paletteStart+4).raw = Color(1.f,1.f,1.f).raw;

		// Alloc tiles
		m_anchor = Vec2u{ 8, 8 };
		auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::LowSpriteBank);
		constexpr auto spriteShape = Sprite::Shape::square16x16;
		constexpr auto numTiles = Sprite::GetNumTiles(spriteShape);
		m_tileNdx = tileBank.allocSTiles(numTiles);

		// Init sprite
		m_sprite = Sprite::ObjectAllocator::alloc(1);
		m_sprite->Configure(Sprite::ObjectMode::Normal, Sprite::GfxMode::Normal, Sprite::ColorMode::e4bits, spriteShape);
		m_sprite->SetNonAffineTransform(false, false, spriteShape);
		m_sprite->setTiles(m_tileNdx, m_paletteStart/16);
		
		m_sprite->setPos(116, 76);

		// Draw into the tiles
		const auto blackNdx = m_paletteStart&0x0f;
		const auto redNdx = blackNdx+1;
		uint32_t twoPixels = blackNdx<<4 | redNdx;
		uint32_t rowA = twoPixels<<24 | twoPixels<<16 | twoPixels<<8 | twoPixels;
		uint32_t rowB = rowA<<4 | blackNdx;

		constexpr uint32_t lowBank = 4;
		for(uint32_t t = 0; t < numTiles; ++t)
		{
			auto& tile = tileBank.GetSTile(m_tileNdx + t);
            renderTile(
                t&1 ? 0.5_p8 : -7.5_p8,
                t&2 ? 0.5_p8 : -7.5_p8,
                tile);
		}
	}

	void renderTile(intp8 x0, intp8 y0, volatile gfx::STile& dst)
	{
		constexpr auto R2 = 8_p8 * 8_p8;
		// Precompute x coordinates
		intp16 x2[8];
		intp16 R2_y2[8];
		intp16 nx2[8];
		intp8 nx[8];
		intp16 ny2[8];
		intp8 ny[8];
		for(int32_t i = 0; i < 8; ++i)
		{
			const auto x = x0+i;
            const auto y = y0 + i;
			x2[i] = x*x;
			nx2[i] = x2[i] / R2.floor();
			nx[i] = sqrt(nx2[i]) * (x < 0_p8? 1 : -1);
			const auto y2 = y*y;
			ny2[i] = y2 / R2.floor();
			ny[i] = sqrt(ny2[i]) * (y < 0_p8 ? 1 : -1);
			R2_y2[i] = R2 - y2;
		}
		// Raster circle
		constexpr auto lx = intp8(1.f/1.41421356237f);
		
		intp16 forwardRow[8] = {};
		for(int row = 0; row < 8; ++row)
		{
			intp16 carry = {};
            uint32_t rowColor = 0;
            for(int i = 7; i >= 0; --i)
            {
                uint32_t pxlColor = 0;
				if(x2[i] < R2_y2[row]) // hit
				{
					auto nz2 = 1_p16 - nx2[i] - ny2[row];
					auto nz = sqrt(nz2);
					auto ndl = saturate((nz+nx[i]+ny[row]) * lx);
					// Color dithering
					pxlColor = m_paletteStart;
					auto dith_x = saturate(ndl + carry + forwardRow[i]);
					auto approx = dith_x.cast<2>();
					intp16 error = dith_x - approx.cast<16>();
					pxlColor += approx.raw;

					carry = error/2;
					forwardRow[(i+7)%8] = forwardRow[(i+7)%8] + carry/2;
					forwardRow[i] = forwardRow[i] + carry/2;
					forwardRow[(i+9)%8] = 0_p16;
				}
                rowColor = rowColor<<4 | pxlColor;
            }
            dst.row(row) = rowColor;
        }
	}

#pragma GCC push_options
#pragma GCC optimize ("O0")
	void update(const Camera& cam)
	{
		Vec3p8 ssPos = cam.projectWorldPos(m_pos);
		int16_t ssX = ssPos.x().roundToInt() - m_anchor.x();
		int16_t ssY = ssPos.y().roundToInt() - m_anchor.y();
		m_sprite->setPos(ssX, ssY);
	}
#pragma GCC pop_options

	void render()
	{}

	math::Vec2u m_anchor;
	math::Vec3p8 m_pos;
    uint32_t m_paletteStart;
	volatile Sprite::Object* m_sprite {};
	uint32_t m_tileNdx = 0;
};

void resetBg2Projection()
{
	REG_BG2PA = 0;
	REG_BG2PC = 0;
	REG_BG2X = -1*(1<<8);
	REG_BG2Y = -1*(1<<8);
}

void InitSystems()
{
	// Init mode7 background
	Display().enableSprites();

	// TextInit
	text.Init();
	
	// enable hblank register and set the mode7 type
	irq_init(NULL);
	irq_add(II_HBLANK, (fnptr)m7_hbl_c);
	// and vblank int for vsync
	irq_add(II_VBLANK, NULL);
}

int main()
{
	Display().StartBlank();
	Display().InitMode2();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	initBackground();

	// -- Init game state ---
	auto camera = Camera(Vec3p8(256_p8, 256_p8, 1.7_p8));

	// Create a 3d object in front of the camera
	Vec3p8 objPos = camera.pos;
	objPos.y() += 5_p8;
	auto obj0 = RasteredObj(objPos);
	
	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		camera.update();
		obj0.update(camera);

		// VSync
		VBlankIntrWait();

		// -- Render --
		// Operations should be ordered from most to least time critical, in case they exceed VBlank time
		// Prepare first scanline for next frame
		resetBg2Projection();
		camera.postGlobalState();

		frameCounter.render(text);
	}
	return 0;
}
