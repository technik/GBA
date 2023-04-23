// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
#include <base.h>

// Engine code
#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Draw.h>
#include <Keypad.h>
#include <linearMath.h>
#include <noise.h>
#include <Text.h>
#include <Timer.h>
#include <gfx/palette.h>
#include <gfx/sprite.h>
#include <gfx/tile.h>
#include <tools/frameCounter.h>

using namespace math;
using namespace gfx;

volatile uint32_t timerT2 = 0;

int main()
{
	// Full resolution, paletized color mode.
	Display().StartBlank();

	// Unlock the display and start rendering
	Display().EndBlank();
	bool vBlank = true;

	// main loop
	int tx = 0;
	while (1)
	{
	}
	return 0;
}
