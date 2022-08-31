#include <raycast.h>

using namespace math;

intp8 rayCast(Vec3p8 rayStart, Vec2p8 rayDir, int& hitVal, int& side, const uint8_t* map, int yStride)
{
	// length of ray from one y-side to next y-side
	int tileX = rayStart.x().floor();
	int stepX; // what direction to step in y-direction (either +1 or -1)
	intp8 deltaDistX;
	intp8 sideDistX = intp8(1<<20); // length of ray from current position to next y-side

	if(rayDir.x() != 0)
	{
		deltaDistX = abs(1_p8 / rayDir.x());
		if (rayDir.x() < 0_p8)
		{
			stepX = -1;
			sideDistX = ((rayStart.x() - tileX) * deltaDistX).cast<8>();
		}
		else
		{
			stepX = 1;
			sideDistX = ((tileX + 1 - rayStart.x()) * deltaDistX).cast<8>();
		}
	}

	// length of ray from one y-side to next y-side
	int tileY = rayStart.y().floor();
	int stepY; // what direction to step in y-direction (either +1 or -1)
	intp8 deltaDistY;
	intp8 sideDistY = intp8(1<<20); // length of ray from current position to next y-side

	if(rayDir.y() != 0)
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