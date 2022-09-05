// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
extern "C"
 {
#include <tonc_types.h>
 }

// Engine code
#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Keypad.h>
#include <linearMath.h>
#include <raycast.h>
#include <Text.h>
#include <Timer.h>
#include <gfx/palette.h>
#include <gfx/sprite.h>
#include <gfx/tile.h>
#include <tools/frameCounter.h>

// Demo code
#include <raycaster.h>
#include <Camera.h>

using namespace math;
using namespace gfx;

TextSystem text;

// No need to place this method in fast memory
void Mode4Renderer::Init()
{
	Mode4Display displayMode;
	displayMode.Init();

	// Initialize the palette
	auto paletteNdx = BackgroundPalette::Allocator::alloc(4);
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::SkyBlue.raw;
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::MidGrey.raw;
	// Wall color
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Green.raw;
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::DarkGreen.raw;
	// Player color
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Yellow.raw;
}

void InitSystems()
{
	Display().enableSprites();
	// TextInit
	text.Init();

	// Set up interrupts
	irq_init(NULL);
	// vblank int for vsync
	irq_add(II_VBLANK, NULL);
}

class MiniMap
{
public:
	MiniMap()
	{
		// Initialize the palette
		m_paletteStart = SpritePalette::Allocator::alloc(2);
		SpritePalette::color(m_paletteStart+0).raw = BasicColor::Black.raw;
		SpritePalette::color(m_paletteStart+1).raw = BasicColor::Green.raw;

		// Init tiles
		auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::HighSpriteBank);
		m_tileNdx = tileBank.allocDTiles(1);

		// Init sprite
		m_Sprite = Sprite::ObjectAllocator::alloc(1);
		m_Sprite->Configure(Sprite::ObjectMode::Normal, Sprite::GfxMode::Normal, Sprite::ColorMode::e16bits, Sprite::Shape::square16x16);
		m_Sprite->SetNonAffineTransform(false, true, Sprite::Shape::square16x16);
		m_Sprite->setPos(240-24, 160-24);
		m_Sprite->setTiles(DTile::HighSpriteBankIndex(m_tileNdx));

		RenderSprite();
	}

	void UpdatePos(unsigned x, unsigned y)
	{
		//
	}

private:
	void RenderSprite()
	{
		auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::HighSpriteBank);
		auto* renderPos = reinterpret_cast<uint16_t*>(&tileBank.GetDTile(m_tileNdx));

		// Fast copy map data into VRAM
		for(int i = 0; i < kMapCols*kMapRows/2; i++)
		{
			uint16_t clrA = g_worldMap[2*i+0] + m_paletteStart;
			uint16_t clrB = g_worldMap[2*i+1] + m_paletteStart;
			renderPos[i] = (clrA << 8) | clrB;
		}
	}

	uint32_t m_paletteStart;
	uint32_t m_tileNdx;
	Sprite::Object* m_Sprite;
};

volatile uint32_t timerT2 = 0;

int main()
{
	// Full resolution, paletized color mode.
	Display().StartBlank();

	// Configure graphics
	Mode4Renderer::Init();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// -- Init game state ---
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(2.5_p8, 2.5_p8, 0_p8));
	auto playerController = CharacterController(camera.m_pose);
	playerController.horSpeed = 0.06125_p8;
	playerController.angSpeed = 0.01_p16;

	MiniMap minimap;

	// Unlock the display and start rendering
	Display().EndBlank();
	bool vBlank = true;

	// main loop
	while(1)
	{
		Timer1().reset<Timer::e64>(); // Set high precision profiler
		// Next frame logic
		Keypad::Update();
		playerController.update();
		playerController.m_pose.pos.x() = max(1.125_p8, playerController.m_pose.pos.x());
		playerController.m_pose.pos.y() = max(1.125_p8, playerController.m_pose.pos.y());
		playerController.m_pose.pos.x() = min(intp8(kMapCols) - 1.125_p8, playerController.m_pose.pos.x());
		playerController.m_pose.pos.y() = min(intp8(kMapRows) - 1.125_p8, playerController.m_pose.pos.y());

		// -- Render --
		Mode4Renderer::RenderWorld(camera);
		frameCounter.render(text);

		timerT2 = Timer1().counter;

		// Present
		if(Keypad::Pressed(Keypad::R))
			vBlank = !vBlank;
		if(vBlank)
			VBlankIntrWait();
			
		Display().flipFrame();

		// Copy the render target
		// TODO: Use the dma here
		// Use DMA channel 3 so that we can copy data from external RAM. If we split the FB into chunks, we may be able to use IWRAM and channel 0.
		//auto backBuffer = (uint32_t*)DisplayControl::Get().backBuffer();
		//DMA::Channel3().Copy(backBuffer, src, Mode4Display::Width * Mode4Display::Height / 4);
	}
	return 0;
}
