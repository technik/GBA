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

float rayCast(Vec3p8 rayStart, Vec2f rayDir, int& hitVal, int& side, uint8_t* map, int yStride)
{
	//calculate step and initial sideDist

	//length of ray from one x-side to next x-side
	float deltaDistX = (rayDir.x() == 0) ? 1e20f : std::abs(1 / rayDir.x());
	int tileX = std::floor((float)rayStart.x());
	float sideDistX; // length of ray from current position to next x-side
	int stepX; //what direction to step in x-direction (either +1 or -1)
	if (rayDir.x() < 0)
	{
		stepX = -1;
		sideDistX = ((float)rayStart.x() - tileX) * deltaDistX;
	}
	else
	{
		stepX = 1;
		sideDistX = (tileX + 1.0 - (float)rayStart.x()) * deltaDistX;
	}

	//length of ray from one y-side to next y-side
	float deltaDistY = (rayDir.y() == 0) ? 1e20f : std::abs(1 / rayDir.y());
	int tileY = std::floor((float)rayStart.y());
	float sideDistY; // length of ray from current position to next y-side
	int stepY; //what direction to step in y-direction (either +1 or -1)
	if (rayDir.y() < 0)
	{
		stepY = -1;
		sideDistY = ((float)rayStart.y() - tileY) * deltaDistY;
	}
	else
	{
		stepY = 1;
		sideDistY = (tileY + 1.0 - (float)rayStart.y()) * deltaDistY;
	}

	//perform DDA
	hitVal = 0;
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
	float hitDistance = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
	return hitDistance;
}

void DrawMinimap(Vec3p8 centerPos)
{
	// TODO: Could probably use a sprite for this
	// That way we could also have rotation

	auto backBuffer = DisplayControl::Get().backBuffer();
	auto startRow = Mode4Display::Width/2 * (Mode4Display::Height - kMapRows);
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

void Render(const Camera& cam)
{
	// Reconstruct local axes for fast ray interpolation
	float cosPhi = cos(float(cam.m_pose.phi) * 6.28f);
	float sinPhi = sin(float(cam.m_pose.phi) * 6.28f);
	Vec2f sideDir = { cosPhi, sinPhi }; // 45deg FoV
	Vec2f viewDir = { -sinPhi, cosPhi };

	for(int col = 0; col < Mode4Display::Width/2; col++)
	{
		float ndcX = 4.f * col / Mode4Display::Width - 1; // screen x from -1 to 1
		// Compute a ray direction for this column
		Vec2f rayDir = viewDir + sideDir * ndcX;

		int cellVal;
		int side;
		const float hitDistance = rayCast(cam.m_pose.pos, rayDir, cellVal, side, worldMap, kMapCols);
		//Calculate height of line to draw on screen
		const int lineHeight = (int)(Mode4Display::Height / hitDistance);

		//calculate lowest and highest pixel to fill in current stripe
		int drawStart = -lineHeight / 2 + Mode4Display::Height / 2;
		if(drawStart < 0)drawStart = 0;
		int drawEnd = lineHeight / 2 + Mode4Display::Height / 2;
		if(drawEnd >= Mode4Display::Height) drawEnd = Mode4Display::Height - 1;

		//draw the pixels of the stripe as a vertical line
      	verLine(col, drawStart, drawEnd, 3+side);
	}

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
		Display().flipFrame();
	}
	return 0;
}
