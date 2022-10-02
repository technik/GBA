//
// mode7.iwram.c
// Interrupts
//
#include <base.h>

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>
#include <raycast.h>

#include <gfx/palette.h>

using namespace math;

uint8_t g_worldMap[kMapRows * kMapCols] = {
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
void Mode4Renderer::yDLine(uint16_t* backBuffer, unsigned x, unsigned drawStart, unsigned drawEnd, uint16_t worldColor)
{
    constexpr unsigned stride = Mode4Display::Width/2;
	// Draw ceiling
	for(unsigned i = 0; i < drawStart; ++i)
	{
		backBuffer[x + i*stride] = 1|(1<<8); // Sky color
	}
	// Draw wall
	for(unsigned i = drawStart; i < drawEnd; ++i)
	{
		backBuffer[x + i*stride] = worldColor; // Wall color
	}
	// Draw ground
	for(unsigned i = drawEnd; i < Mode4Display::Height; ++i)
	{
		backBuffer[x + i*stride] = 2|(2<<8); // Ground color
	}
}

// Draw a pixel line with a single pixel line
void yLine(Color* backBuffer,
	unsigned x,
	unsigned drawStart, unsigned drawEnd,
	Color skyColor, Color wallColor, Color groundColor)
{
    constexpr unsigned stride = Mode3Display::Width;
	// Draw ceiling
	for(int i = 0; i < drawStart; ++i)
	{
		backBuffer[x + i*stride] = skyColor;
	}
	// Draw wall
	for(int i = drawStart; i < drawEnd; ++i)
	{
		backBuffer[x + i*stride] = wallColor; // Wall color
	}
	// Draw ground
	for(int i = drawEnd; i < Mode4Display::Height; ++i)
	{
		backBuffer[x + i*stride] = groundColor; // Ground color
	}
}

/* Deprecated
void Mode4Renderer::RenderWorld(const Camera& cam)
{	
	// Reconstruct local axes for fast ray interpolation
	intp8 cosPhi = cam.m_pose.cosf.cast<8>();
	intp8 sinPhi = cam.m_pose.sinf.cast<8>();
	Vec2p8 sideDir = { cosPhi, sinPhi }; // 45deg FoV
	Vec2p8 viewDir = { -sinPhi, cosPhi };

	// TODO: We can leverage the fact that we're now multiplying by col only and transform the in-loop multiplication into an addition.
	// On top of that, sideDir.x() * ndcX can really be extracted and transformed into two separate additions too.
	// This should remove two two muls and to casts per loop.
	constexpr intp8 widthRCP = intp8(4.f/(Mode4Display::Width-1));
	auto backbuffer = DisplayControl::Get().backBuffer();

	const uint16_t colorOffset = sPaletteStart | (sPaletteStart<<8);
	const int16_t wallDColorSeam = (4 | (4<<8)) + colorOffset;
	const int16_t wallDColorDark = (5 | (5<<8)) + colorOffset;
	const int16_t wallDColorLight =(6 | (6<<8)) + colorOffset;

	Vec2p8 rayDir0 = viewDir - sideDir;
	Vec2p12 dRay = { (sideDir.x() * widthRCP).cast<12>(), (sideDir.y() * widthRCP).cast<12>() };

	for(int col = 0; col < Mode4Display::Width/2; col++)
	{
		// Compute a ray direction for this column
		Vec2p8 rayDir = { 
			rayDir0.x() + (col * dRay.x()).cast<8>(),
			rayDir0.y() + (col * dRay.y()).cast<8>()
		};

		int cellVal;
		int side;
		const intp8 hitDistance = rayCast(cam.m_pose.pos, rayDir, cellVal, side, g_worldMap, kMapCols);
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

		// Wall textures
		Vec2p8 hitPoint = Vec2p8(cam.m_pose.pos.x(), cam.m_pose.pos.y()) + Vec2p8((hitDistance * rayDir.x()).cast<8>(), (hitDistance * rayDir.y()).cast<8>());
		int texX = ((side ? hitPoint.x() : hitPoint.y()).raw >> 4) & 0xf;

		auto texClr = side ? wallDColorDark : wallDColorLight;

		//draw the pixels of the stripe as a vertical line
		yDLine(backbuffer, col, drawStart, drawEnd, texX ? texClr : wallDColorSeam);
	}
}*/

void DrawMinimapMode3(Color* backBuffer, Vec3p8 centerPos)
{
	// TODO: Could probably use a sprite for this
	// That way we could also have rotation

	//auto backBuffer = DisplayControl::Get().backBuffer();
	auto startRow = Mode3Display::Width * (Mode3Display::Height - kMapRows - 1 - 4);
	auto pixelOffset = startRow + (Mode3Display::Width - kMapCols - 4);
	auto dst = &backBuffer[pixelOffset];

	// Minimap center
	int tileX = centerPos.x().floor();
	int tileY = centerPos.y().floor();

	for(int y = 0; y < kMapRows; y++)
	{
		for(int x = 0; x < kMapCols; x++)
		{
			auto clr = g_worldMap[x+kMapCols*y] ? BasicColor::Green : BasicColor::Black;

			// Write
			dst[(x + Mode4Display::Width*(kMapRows-y))] = clr;
		}
	}

	// Draw the character
	dst[(tileX + Mode3Display::Width*(kMapRows-tileY))] = BasicColor::Yellow;
}

/* Deprecated
void RenderMode3(const Camera& cam)
{	
	auto backBuffer = Mode3Display::frontBuffer();// DisplayControl::Get().backBuffer();
	// Reconstruct local axes for fast ray interpolation
	intp8 cosPhi = cam.m_pose.cosf.cast<8>();
	intp8 sinPhi = cam.m_pose.sinf.cast<8>();
	Vec2p8 sideDir = { cosPhi, sinPhi }; // 45deg FoV
	Vec2p8 viewDir = { -sinPhi, cosPhi };

	// TODO: We can leverage the fact that we're now multiplying by col only and transform the in-loop multiplication into an addition.
	// On top of that, sideDir.x() * ndcX can really be extracted and transformed into two separate additions too.
	// This should remove two two muls and to casts per loop.
	constexpr intp8 widthRCP = intp8(2.f/Mode3Display::Width);

	//intp8 ndcX = -1_p8;

	Vec2p8 rayDir = viewDir - sideDir;
	Vec2p8 dRay = { (sideDir.x() * widthRCP).cast<8>(), (sideDir.y() * widthRCP).cast<8>() };

	for(int col = 0; col < Mode3Display::Width; col++)
	{
		int cellVal;
		int side;
		const intp8 hitDistance = rayCast(cam.m_pose.pos, rayDir, cellVal, side, g_worldMap, kMapCols);
		//Calculate height of line to draw on screen
		int lineHeight = Mode3Display::Height;
		if(hitDistance > 0_p8) // This could really be > 1, as it will saturate to full screen anyway for distances < 1
		{
			lineHeight = (intp8(Mode3Display::Height ) / hitDistance).floor();
		}

		//calculate lowest and highest pixel to fill in current stripe
		int drawStart = -lineHeight / 2 + Mode3Display::Height / 2;
		if(drawStart < 0)drawStart = 0;
		int drawEnd = lineHeight / 2 + Mode3Display::Height / 2;
		if(drawEnd > Mode3Display::Height) drawEnd = Mode3Display::Height;

		//draw the pixels of the stripe as a vertical line
		yLine(backBuffer,
			col, drawStart, drawEnd,
			BasicColor::SkyBlue, side ? BasicColor::DarkGreen : BasicColor::Green, BasicColor::MidGrey);

		rayDir += dRay;
	}

	// Measure render time
	DrawMinimapMode3(backBuffer, cam.m_pose.pos);
}*/