//
// mode7.iwram.c
// Interrupts
//
extern "C" {
	#include <tonc.h>
}

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>
#include <raycast.h>

#include <gfx/palette.h>

using namespace math;

constexpr uint8_t worldMap[kMapRows * kMapCols] = {
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
void verLine(uint16_t* backBuffer, unsigned x, unsigned drawStart, unsigned drawEnd, uint16_t worldColor)
{
	int16_t dPxl = worldColor | (worldColor<<8);
    constexpr unsigned stride = Mode4Display::Width/2;
	// Draw ceiling
	for(int i = 0; i < drawStart; ++i)
	{
		backBuffer[x + i*stride] = 1|(1<<8); // Sky color
	}
	// Draw wall
	for(int i = drawStart; i < drawEnd; ++i)
	{
		backBuffer[x + i*stride] = dPxl; // Wall color
	}
	// Draw ground
	for(int i = drawEnd; i < Mode4Display::Height; ++i)
	{
		backBuffer[x + i*stride] = 2|(2<<8); // Ground color
	}
}

void DrawMinimap(uint16_t* backBuffer, Vec3p8 centerPos)
{
	// TODO: Could probably use a sprite for this
	// That way we could also have rotation

	//auto backBuffer = DisplayControl::Get().backBuffer();
	auto startRow = Mode4Display::Width/2 * (Mode4Display::Height - kMapRows - 1 - 4);
	auto pixelOffset = startRow + (Mode4Display::Width - kMapCols - 4) / 2;
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
	intp8 cosPhi = cam.m_pose.cosf.cast<8>();
	intp8 sinPhi = cam.m_pose.sinf.cast<8>();
	Vec2p8 sideDir = { cosPhi, sinPhi }; // 45deg FoV
	Vec2p8 viewDir = { -sinPhi, cosPhi };

	// TODO: We can leverage the fact that we're now multiplying by col only and transform the in-loop multiplication into an addition.
	// On top of that, sideDir.x() * ndcX can really be extracted and transformed into two separate additions too.
	// This should remove two two muls and to casts per loop.
	constexpr intp8 widthRCP = intp8(4.f/Mode4Display::Width);

	intp8 ndcX = -1_p8;
	for(int col = 0; col < Mode4Display::Width/2; col++)
	{
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
		if(hitDistance > 0_p8) // This could really be > 1, as it will saturate to full screen anyway for distances < 1
		{
			lineHeight = (intp8(Mode4Display::Height ) / hitDistance).floor();
		}

		//calculate lowest and highest pixel to fill in current stripe
		int drawStart = -lineHeight / 2 + Mode4Display::Height / 2;
		if(drawStart < 0)drawStart = 0;
		int drawEnd = lineHeight / 2 + Mode4Display::Height / 2;
		if(drawEnd > Mode4Display::Height) drawEnd = Mode4Display::Height;

		//draw the pixels of the stripe as a vertical line
      	verLine(DisplayControl::Get().backBuffer(), col, drawStart, drawEnd, 3+side);
		ndcX += widthRCP; // screen x from -1 to 1
	}

	// Measure render time
	DrawMinimap(DisplayControl::Get().backBuffer(), cam.m_pose.pos);
}