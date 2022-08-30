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
using namespace gfx;

TextSystem text;

struct Sphere
{
	Vec3p8 pos;
	intp8 radius;
};

void initBackgroundPalette()
{
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

constexpr int kCellSize = 64;
constexpr int kMapRows = 16;
constexpr int kMapCols = 16;
uint8_t worldMap[kMapRows * kMapCols] = {
	1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
	1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1,
	1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1,
	1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1,
};

// Actually draws two pixels at once
void verLine(int x, int drawStart, int drawEnd, int worldColor)
{
	int16_t dPxl = worldColor | (worldColor<<8);
	auto backBuffer = DisplayControl::Get().backBuffer();
	// Draw ceiling
	for(int i = 0; i < drawStart; ++i)
	{
		backBuffer[x + i*Mode4Display::Width/2] = 1|(1<<8); // Sky color
	}
	// Draw wall
	for(int i = drawStart; i < drawEnd; ++i)
	{
		backBuffer[x + i*Mode4Display::Width/2] = dPxl; // Wall color
	}
	// Draw ground
	for(int i = drawEnd; i < Mode4Display::Height; ++i)
	{
		backBuffer[x + i*Mode4Display::Width/2] = 2|(2<<8); // Ground color
	}
}

intp8 rayCast(Vec3p8 rayStart, Vec2p8 rayDir, int& hitVal, int& side, uint8_t* map, int yStride)
{
	// length of ray from one x-side to next x-side
	float deltaDistXfloat = (rayDir.x() == 0_p8) ? 1e10f : std::abs(1.f / (float)rayDir.x());
	int tileX = rayStart.x().floor();
	intp8 sideDistX; // length of ray from current position to next x-side
	int stepX; // what direction to step in x-direction (either +1 or -1)
	if (rayDir.x() < 0_p8)
	{
		stepX = -1;
		sideDistX = ((rayStart.x() - tileX) * intp12(deltaDistXfloat)).cast<8>();
	}
	else
	{
		stepX = 1;
		sideDistX = intp8(float((tileX + 1 - rayStart.x())) * deltaDistXfloat);
	}

	// length of ray from one y-side to next y-side
	int tileY = rayStart.y().floor();
	int stepY; // what direction to step in y-direction (either +1 or -1)
	intp8 deltaDistY;
	intp8 sideDistY; // length of ray from current position to next y-side

	if(rayDir.y() == 0)
	{
		stepY = 0;
		deltaDistY = 1e10_p8; // Very high number. Overflow danger!
		sideDistY = 1e20_p8; // Make sure this is always the largest distance.
	}
	else
	{
		deltaDistY = abs(1_p8 / rayDir.y());
		if (rayDir.y() < 0_p8)
		{
			stepY = -1;
			sideDistY = ((rayStart.y() - tileY) * deltaDistY).cast<8>();
		}
		else
		{
			stepY = 1;
			sideDistY = ((tileY + 1 - rayStart.y()) * deltaDistY).cast<8>();
		}
	}

	//perform DDA
	hitVal = 0;
	intp8 deltaDistX = intp8(deltaDistXfloat);
	while (hitVal == 0)
	{
		//jump to next map square, either in x-direction, or in y-direction
		if (sideDistX < sideDistY)
		{
			sideDistX += deltaDistX;
			tileX += stepX;
			side = 0;
		}
		else
		{
			sideDistY += deltaDistY;
			tileY += stepY;
			side = 1;
		}
		//Check if ray has hit a wall
		hitVal = map[tileX + yStride * tileY];
	}


	//Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
	intp8 hitDistance = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
	return hitDistance;
}

void DrawMinimap(Vec3p8 centerPos)
{
	// TODO: Could probably use a sprite for this
	// That way we could also have rotation

	auto backBuffer = DisplayControl::Get().backBuffer();
	auto startRow = Mode4Display::Width/2 * (Mode4Display::Height - kMapRows - 1);
	auto pixelOffset = startRow + (Mode4Display::Width - kMapCols) / 2;
	auto dst = &backBuffer[pixelOffset];

	// Minimap center
	int tileX = centerPos.x().floor();
	int tileY = centerPos.y().floor();

	for(int y = 0; y < kMapRows; y++)
	{
		for(int x = 0; x < kMapCols; x++)
		{
			uint16_t clr = worldMap[x+kMapCols*y] ? 3 : 0;
			++x;
			clr |= (worldMap[x+kMapCols*y] ? 3 : 0) << 8;

			// Write
			dst[(x + Mode4Display::Width*(kMapRows-y))/2] = clr;
		}
	}

	// Draw the character
	auto base = dst[(tileX + Mode4Display::Width*(kMapRows-tileY))/2];
	if(tileX & 1)
	{
		base = (base & 0x0f) | (5<<8);
	}
	else
	{
		base = (base & 0xf0) | 5;
	}
	dst[(tileX + Mode4Display::Width*(kMapRows-tileY))/2] = base;
}

volatile uint32_t timerT = 0;

void Render(const Camera& cam)
{
	// Reconstruct local axes for fast ray interpolation
	float cosPhi = cos(float(cam.m_pose.phi) * 6.28f);
	float sinPhi = sin(float(cam.m_pose.phi) * 6.28f);
	Vec2p8 sideDir = { intp8(cosPhi), intp8(sinPhi) }; // 45deg FoV
	Vec2p8 viewDir = { -intp8(sinPhi), intp8(cosPhi) };

	// Profiling
	Timer1().reset<Timer::e256>(); // Set high precision profiler

	for(int col = 0; col < Mode4Display::Width/2; col++)
	{
		intp8 ndcX = (intp8(4 * col) * intp12::castFromShiftedInteger<16>(lu_div(Mode4Display::Width))).cast<8>() - 1_p8; // screen x from -1 to 1
		// Compute a ray direction for this column
		Vec2p8 rayDir = { 
			viewDir.x() + (sideDir.x() * ndcX).cast<8>(),
			viewDir.y() + (sideDir.y() * ndcX).cast<8>()
		};

		int cellVal;
		int side;
		const intp8 hitDistance = rayCast(cam.m_pose.pos, rayDir, cellVal, side, worldMap, kMapCols);
		//Calculate height of line to draw on screen
		int lineHeight = Mode4Display::Height;
		if(hitDistance > 0_p8)
		{
			lineHeight = (intp8(Mode4Display::Height ) / hitDistance).floor();
		}

		//calculate lowest and highest pixel to fill in current stripe
		int drawStart = -lineHeight / 2 + Mode4Display::Height / 2;
		if(drawStart < 0)drawStart = 0;
		int drawEnd = lineHeight / 2 + Mode4Display::Height / 2;
		if(drawEnd >= Mode4Display::Height) drawEnd = Mode4Display::Height - 1;

		//draw the pixels of the stripe as a vertical line
      	verLine(col, drawStart, drawEnd, 3+side);
	}

	// Measure render time
	timerT = Timer1().counter;
	DrawMinimap(cam.m_pose.pos);
}

int main()
{
	// Full resolution, paletized color mode.
	Mode4Display mode4;
	mode4.Init();
	Display().StartBlank();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	initBackgroundPalette();

	// -- Init game state ---
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(2.5_p8, 2.5_p8, 0_p8));
	auto playerController = CharacterController(camera.m_pose);
	playerController.horSpeed = 0.125_p8;
	playerController.angSpeed = 0.02_p16;

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		Keypad::Update();
		playerController.update();
		playerController.m_pose.pos.x() = max(1.125_p8, playerController.m_pose.pos.x());
		playerController.m_pose.pos.y() = max(1.125_p8, playerController.m_pose.pos.y());
		playerController.m_pose.pos.x() = min(intp8(kMapCols) - 1.125_p8, playerController.m_pose.pos.x());
		playerController.m_pose.pos.y() = min(intp8(kMapRows) - 1.125_p8, playerController.m_pose.pos.y());

		// -- Render --
		Render(camera);
		frameCounter.render(text);

		// Present
		VBlankIntrWait();
		Display().flipFrame();
	}
	return 0;
}
