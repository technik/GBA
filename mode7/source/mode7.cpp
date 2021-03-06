// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
#include <cstring>

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
#include <gfx/armGfx.h>
#include <tools/frameCounter.h>

// Demo code
#include <Camera.h>

// Inline data
#include <poolMap.h>

using namespace math;

TextSystem text;

void postGlobalState(const Pose& cam)
{
	// Copy local state into global variables that can be accessed by the renderer
	PostCameraState(cam.pos, cam.cosf, cam.sinf);
}

void SetBackgroundClearColor(Color clr)
{
	gfx::BackgroundPalette::color(0).raw = clr.raw;
}

struct Mode7Bg
{
	void Init(
	const uint32_t paletteSize, const uint32_t* paletteData,
	const uint32_t tileDataSize, const uint32_t* tileData,
	const uint32_t mapDataSize, const uint32_t* mapData)
	{
		// Load palette
		memcpy(gfx::BackgroundPalette::rawMemory(), paletteData, paletteSize*4);

		// Load the background tile map
		auto numTiles = tileDataSize * 4 / sizeof(gfx::DTile);
		auto& tileBank = gfx::TileBank::GetBank(0);
		auto tileStart = tileBank.allocDTiles(numTiles);
		
		memcpy(tileBank.memory(), tileData, tileDataSize*4);

		// Config BG2
		// Use charblock 0 for the tiles
		// Use the first screen block after charblock 0 (i.e. screenblock 8)
		// 128*128 map size
		IO::BG2CNT::Get().value =
			(1<<7) | // 16 bit color, technically not necessary as Affine backgrounds are always 16bit colors
			(8<<8) | // screenblock 8
			(2<<0xe); // size 512x512

		// Fill in map data
		// Affine maps use 8 bit indices
		auto* mapMem = reinterpret_cast<uint16_t*>(VideoMemAddress+0x4000);
		memcpy(mapMem, mapData, mapDataSize*4);
	}

	void End()
	{
		// Invalidate all allocated tiles
		auto& tileBank = gfx::TileBank::GetBank(0);
		tileBank.reset();
	}
};

struct Billboard
{
	static constexpr auto ambientColor = BasicColor::Blue;
	static constexpr auto litColor = BasicColor::White;

	Billboard()
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

		// Alloc transform
		m_transformId = Sprite::TransformAllocator::alloc(1);

		// Init sprite
		m_sprite = Sprite::ObjectAllocator::alloc(1);
		m_sprite->Configure(Sprite::ObjectMode::Affine2x, Sprite::GfxMode::Normal, Sprite::ColorMode::e4bits, spriteShape);
		//m_sprite->SetNonAffineTransform(false, false, spriteShape);
		m_sprite->SetAffineConfig(m_transformId, spriteShape);
		m_sprite->setTiles(m_tileNdx, m_paletteStart/16);
		
		m_sprite->setPos(116, 76);

		// Draw into the tiles
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

	void update(const Camera& cam)
	{
		const int32_t kScale = 8;
		Vec3p8 ssPos = cam.projectWorldPos(m_pose.pos);
		m_ssX = ssPos.x().roundToInt() - m_anchor.x();
		m_ssY = ssPos.y().roundToInt() - m_anchor.y();
		auto rightSide = m_ssX + 16;
		auto bottomSide = m_ssY + 16;  
		auto depth = ssPos.z();
		if(depth > 0.1_p8
		 && rightSide >= 0
		 && m_ssX <= ScreenWidth
		 && bottomSide >= 0
		 && m_ssY <= ScreenWidth)
		{
			m_scale.raw = int16_t(depth.raw/kScale);
			m_ssY = ssPos.y().roundToInt() - m_anchor.y() - 16;
			m_visible = true;
		}
		else
		{
			m_visible = false;
		}
	}

	void render(const Camera& cam)
	{
		if(m_visible)
		{
			m_sprite->setPos(m_ssX, m_ssY);
			m_sprite->show(Sprite::ObjectMode::Affine2x);
		}
		else
		{
			m_sprite->hide();
		}

		auto& tx = Sprite::OAM_Transforms()[m_transformId];
		tx.pa = m_scale.raw;
		tx.pd = m_scale.raw;
	}

	bool m_visible = true;
	int8p8 m_scale;
	int16_t m_ssX, m_ssY;
	math::Vec2u m_anchor;
    uint32_t m_paletteStart;
	volatile Sprite::Object* m_sprite {};
	uint32_t m_tileNdx = 0;
	int32_t m_transformId;
	Pose m_pose;
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
	Display().SetMode<2,DisplayControl::BG2>();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	Mode7Bg mode7bg;
	mode7bg.Init(
		mapPaletteSize, mapPalette,
		bgTilesSize, bgTiles,
		mapDataSize, mapData
	);
	SetBackgroundClearColor(BasicColor::SkyBlue); // Override Background clear color (used for blending too)

	// -- Init game state ---
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(26_p8, 15_p8, 3_p8));

	// Create a 3d object in front of the camera
	auto obj0 = Billboard();
	obj0.m_pose.pos = Vec3p8(26_p8, 20_p8, 1_p8);
	auto playerController = CharacterController(obj0.m_pose);
	auto follower = PoseFollower(obj0.m_pose, Vec3p8(0_p8, -5_p8, 1.5_p8));
	
	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		Keypad::Update();
		playerController.update();
		follower.update();
		camera.m_pose = follower.m_pose;
		obj0.update(camera); // Prepare for render

		// VSync
		VBlankIntrWait();
		obj0.render(camera);

		// -- Render --
		// Operations should be ordered from most to least time critical, in case they exceed VBlank time
		// Prepare first scanline for next frame
		resetBg2Projection();
		postGlobalState(camera.m_pose);

		frameCounter.render(text);
	}
	return 0;
}
