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

void initBackground()
{
	// Initialize the palette
	auto paletteNdx = BackgroundPalette::Allocator::alloc(2);
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Red.raw;
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Blue.raw;
}

void InitSystems()
{
	Display().enableSprites();
	// TextInit
	text.Init();
}

constexpr int kCellSize = 64;
constexpr int kMapRows = 8;
constexpr int kMapCols = 8;
uint8_t worldMap[kMapRows * kMapCols] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 1, 1, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
};

// Actually draws two pixels at once
void verLine(int x, int drawStart, int drawEnd)
{
	auto backBuffer = DisplayControl::Get().backBuffer();
	// Draw ceiling
	for(int i = 0; i < drawStart; ++i)
	{
		backBuffer[x + i*Mode4Display::Width/2] = 0; // Black
	}
	// Draw wall
	for(int i = drawStart; i < drawEnd; ++i)
	{
		backBuffer[x + i*Mode4Display::Width/2] = 1|(1<<8); // Wall color
	}
	// Draw ground
	for(int i = drawEnd; i < Mode4Display::Height; ++i)
	{
		backBuffer[x + i*Mode4Display::Width/2] = 0; // Black
	}
}

void Render(const Camera& cam)
{
	// Reconstruct local axes for fast ray interpolation
	float cosPhi = cos(float(cam.m_pose.phi));
	float sinPhi = sin(float(cam.m_pose.phi));
	Vec2f sideDir = { cosPhi/2, sinPhi/2 }; // 45deg FoV
	Vec2f viewDir = { -sinPhi, cosPhi };

	for(int col = 2; col < Mode4Display::Width/2 - 2; col++)
	{
		float ndcX = 2.f * col / Mode4Display::Width - 1; // screen x from -1 to 1
		// Compute a ray direction for this column
		Vec2f rayDir = viewDir + sideDir * ndcX;

		int tileX = std::floor((float)cam.m_pose.pos.x());
		int tileY = std::floor((float)cam.m_pose.pos.y());

		//length of ray from current position to next x or y-side
      	float sideDistX;
        float sideDistY;

        //length of ray from one x or y-side to next x or y-side
        float deltaDistX = (rayDir.x() == 0) ? 1e20f : std::abs(1 / rayDir.x());
        float deltaDistY = (rayDir.y() == 0) ? 1e20f : std::abs(1 / rayDir.y());
		double perpWallDist;

		//what direction to step in x or y-direction (either +1 or -1)
		int stepX;
		int stepY;

		int hit = 0; //was there a wall hit?
		int side; //was a NS or a EW wall hit?

		//calculate step and initial sideDist
		if (rayDir.x() < 0)
		{
			stepX = -1;
			sideDistX = ((float)cam.m_pose.pos.x() - tileX) * deltaDistX;
		}
		else
		{
			stepX = 1;
			sideDistX = (tileX + 1.0 - (float)cam.m_pose.pos.x()) * deltaDistX;
		}
		if (rayDir.y() < 0)
		{
			stepY = -1;
			sideDistY = ((float)cam.m_pose.pos.y() - tileY) * deltaDistY;
		}
		else
		{
			stepY = 1;
			sideDistY = (tileY + 1.0 - (float)cam.m_pose.pos.y()) * deltaDistY;
		}

		//perform DDA
		while (hit == 0)
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
			if (worldMap[tileX + kMapCols * tileY] > 0)
			{
				hit = 1;
			}
		}

		//Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
		if(side == 0)
		{
			perpWallDist = (sideDistX - deltaDistX);
		}
		else
		{
			perpWallDist = (sideDistY - deltaDistY);
		}

		//Calculate height of line to draw on screen
		int lineHeight = (int)(Mode4Display::Height / perpWallDist);

		//calculate lowest and highest pixel to fill in current stripe
		int drawStart = -lineHeight / 2 + Mode4Display::Height / 2;
		if(drawStart < 0)drawStart = 0;
		int drawEnd = lineHeight / 2 + Mode4Display::Height / 2;
		if(drawEnd >= Mode4Display::Height) drawEnd = Mode4Display::Height - 1;

		//draw the pixels of the stripe as a vertical line
      	verLine(col, drawStart, drawEnd);
	}
}

int main()
{
	Display().StartBlank();

	// Full resolution, paletized color mode.
	Mode4Display mode4;
	mode4.Init();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	initBackground();

	// -- Init game state ---
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(2.5_p8, 2.5_p8, 0_p8));
	auto playerController = CharacterController(camera.m_pose);
	playerController.horSpeed = 0.02_p8;
	playerController.angSpeed = 0.125_p8;

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		Keypad::Update();
		playerController.update();

		// -- Render --
		Render(camera);
		frameCounter.render(text);

		// Present
		Display().flipFrame();
	}
	return 0;
}
