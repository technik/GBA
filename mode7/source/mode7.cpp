// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
#include <tonc.h>

// Engine code
#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Keypad.h>
#include <linearMath.h>
#include <Text.h>
#include <Timer.h>
#include <tiles.h>

// Demo code
#include <demo.h>
#include <Camera.h>

// Global Camera State
VECTOR gCamPos;
FIXED gCosf = 1<<8;
FIXED gSinf = 0;

auto allocSprites(uint32_t n=1)
{
	static uint32_t count = 0;
	auto pos = count;
	count += n;
	return pos;
}

// === FUNCTIONS ======================================================
struct FrameCounter
{
	auto& tile(uint32_t n)
	{
		auto ndx = m_spriteNdx + n;
		auto* obj = &Sprite::OAM()[ndx/4].objects[ndx%4];
		return *obj;
	}

	FrameCounter()
		: m_spriteNdx(allocSprites(2))
	{
		// Init the tiles
		for(uint32_t i = 0; i < 2; ++i)
		{
			auto& obj = tile(i);
			obj.attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
			obj.attribute[1] = 8*i; // Left of the screen, small size
			obj.attribute[2] = Sprite::DTile::HighSpriteBankIndex('0'-32);
		}
	}

	void render()
	{
		// Separate digits
		auto fps = count();
		auto fps10 = fps/10;

		// Draw frame rate indicator
		tile(0).attribute[2] = Sprite::DTile::HighSpriteBankIndex(fps10+16);
		tile(1).attribute[2] = Sprite::DTile::HighSpriteBankIndex(fps-10*fps10+16);
	}

	uint32_t count()
	{
		//uint32_t ms = Timer0().counter/16; // ~Milliseconds
		uint32_t tc = Timer0().counter;
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
		uint32_t fps = 60;
		if(tc > 16*256+3*64) // ~16.75, crude approx for 16.6667
		{
			fps = 30;
			if(tc > (33*256+128)) // 33.5 ms
			{
				fps = 20;
				if(tc > 50*256) // 66.75 ~= 66.6667
				{
					fps=15;
					if(tc > 66*256+3*64)
					{
						fps = 10;
						if(fps > 100*256)
						{
							fps = 0; // Slower than 10fps, don't bother
						}
					}
				}
			}
		}
		return fps;
	}

	const uint32_t m_spriteNdx;
};

void initBackground()
{
	// Background clear color (used for blending too)
	BackgroundPalette()[0].raw = BasicColor::SkyBlue.raw;
	// Prepare the background tile map
	BackgroundPalette()[1].raw = BasicColor::White.raw;
	BackgroundPalette()[2].raw = BasicColor::Red.raw;

	// Config BG2
	// Use charblock 0 for the tiles
	// Use the first screen block after charblock 0 (i.e. screenblock 8)
	// 128*128 map size
	IO::BG2CNT::Get().value =
		(1<<7) | // 16 bit color
		(8<<8) | // screenblock 8
		(3<<0xe); // size 1024*1024

	// Fill in a couple tiles in video memory
    auto& tile0 = Sprite::DTileBlock(0)[0];
	tile0.fill(1); // White
	auto& tile1 = Sprite::DTileBlock(0)[1];
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
	auto& obj()
	{
		auto ndx = m_spriteNdx;
		return Sprite::OAM()[ndx/4].objects[ndx%4];
	}

	RasteredObj(VECTOR startPos)
		:m_pos(startPos)
	{
		m_paletteStart = SpritePaletteAllocator::alloc(1); // Alloc 1 color

		// Init palette
		SpritePalette()[m_paletteStart].raw = BasicColor::Green.raw;

		// Init sprite
		m_spriteNdx = allocSprites(1);
		m_tileNdx = SpriteTileAllocator::alloc(SpriteTileAllocator::Bank::Low, 1);
		auto& sprite = obj();
		sprite.attribute[2] = m_tileNdx;
		sprite.setPos(116, 76);

		// Draw into the tile
		constexpr uint32_t lowBank = 4;
		auto* spriteBase = &((volatile uint16_t*)Sprite::STileBlock(lowBank))[m_tileNdx];
		uint32_t twoTiles = (m_paletteStart<<8) | m_paletteStart;
		for(uint16_t i = 0; i < 8*4; ++i) // Fill the tile
		{
			spriteBase[i] = (i&2) ? twoTiles : (twoTiles<<4);
		}
	}

	void update(const Camera& cam)
	{
		VECTOR relPos;
		relPos.x = m_pos.x-cam.pos.x;
		relPos.y = m_pos.y-cam.pos.y;
		relPos.z = m_pos.z-cam.pos.z;
		//VECTOR vsPos = relPos.x*
	}

	void render()
	{}

	VECTOR m_pos;
    uint32_t m_paletteStart;
	uint32_t m_spriteNdx = 0;
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
	initBackground();
	Display().enableSprites();

	// TextInit
	TextSystem text;
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
	FrameCounter frameCounter;

	// -- Init game state ---
	auto camera = Camera({ 256<<8, 256<<8, 2<<8 });

	// Create a 3d object in front of the camera
	VECTOR objPos = camera.pos;
	objPos.y += 5;
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
		resetBg2Projection();
		camera.postGlobalState();
		// Prepare first scanline for next frame

		frameCounter.render();
	}
	return 0;
}
