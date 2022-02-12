#include <cstdint>
#include <linearMath.h>
#include <Display.h>
#include <Device.h>
#include <Draw.h>
#include <Timer.h>
#include <cmath>
#include <tiles.h>
#include <Text.h>
#include <Keypad.h>
#include <tonc_math.h>
#include <matrix.h>

using namespace math;

//#define EWRAM_DATA __attribute__((section(".ewram")))
//#define IWRAM_DATA __attribute__((section(".iwram")))
//#define  EWRAM_BSS __attribute__((section(".sbss")))
//
//#define EWRAM_CODE __attribute__((section(".ewram"), long_call))
//#define IWRAM_CODE __attribute__((section(".iwram"), long_call))

typedef void (*fnptr)(void);
//#define REG_ISR_MAIN *(volatile int32_t*)(0x03007FFC)
#define REG_ISR_MAIN *(volatile fnptr*)(0x03007FFC)

//__attribute__((section(".iwram"))) Mat22p12 gScanlineTransforms[ScreenHeight];
Mat22p12 gScanlineTransforms[ScreenHeight];


intp12 PlaneScanlineIntersection(intp12 h, int32_t scanline);
void FillInvProjMatrix(intp12 cx, intp12 cy, intp12 cz, intp12 depth, Mat22p12& dst);

void RefreshAffineTransforms(Vec3p12 camPos)
{
	// Out of screen, start computing all the affine matrices
	for(int32_t i = 0; i < ScreenHeight; ++i)
	{
		auto d = PlaneScanlineIntersection(camPos.z(), i);
		FillInvProjMatrix(0_p12, 0_p12, camPos.z(), d, gScanlineTransforms[i]);
	}
}

extern "C" {
	void hblank_cb() __attribute__ ((interrupt ("IRQ"), section(".iwram"), long_call));
}

void hblank_cb()
{
	uint16_t vcount = IO::VCOUNT::Get().value + 1;
	BackgroundPalette()[1].raw = (vcount&1) ? BasicColor::Blue.raw : BasicColor::Green.raw;
	if(vcount < 160) // Need to update the next projection matrix
	{
		auto& tx = gScanlineTransforms[vcount];

		// u,v offsets
		IO::BG2P::Get().refPoint.x() = tx.m[0][1].cast_down<8>();
		IO::BG2P::Get().refPoint.y() = tx.m[1][1].cast_down<8>();
		
		// Scale/rotation
		IO::BG2P::Get().A = int16_t(tx.m[0][0].cast_down<8>().raw);
		// These two are zero anyway
		//IO::BG2P::Get().B = 0;
		//IO::BG2P::Get().C = 0;
		IO::BG2P::Get().D = int16_t(tx.m[1][0].cast_down<8>().raw);
	}

	IO::IF::Get().value = (1<<1); // Clear the interrupt flag
}

void plotFrameIndicator()
{
	// Draw frame rate indicator
	uint32_t ms = Timer0().counter/16; // ~Milliseconds
	auto* tile0 = &Sprite::OAM()[0].objects[0];
	auto ms10 = ms/10;
	tile0->attribute[2] = Sprite::DTile::HighSpriteBankIndex(ms10+16);
	auto* tile1 = &Sprite::OAM()[0].objects[1];
	tile1->attribute[2] = Sprite::DTile::HighSpriteBankIndex(ms-10*ms10+16);
}

// Constants to transform from world to texture space
constexpr uint32_t TexelsPerMeter = 8;
constexpr uint32_t TexelOffset = 128/2; // Center the texture at the world origin.

// Inverse projection data to transform from screen space (pixel coordinates, scanlines)
// To view space (camera aligned, 3d coordinates)
#define FOV_Y_90
#ifdef FOV_Y_90
constexpr intp12 tgY = 1_p12;
constexpr intp12 tgX = 1.5_p12;
#elif defined(FOV_X_90)
constexpr intp12 tgY = 0.6666666666666667_p12;
constexpr intp12 tgX = 1_p12;
#elif defined(FOV_Y_45)
constexpr intp12 tgY = 0.41421356237309503_p12;
constexpr intp12 tgX = 0.6213203435596426_p12;
#endif

// Returns the intersection depth of the plane defined by a scanline and the view origin,
// with a horizontal plane at height h below the camera.
// Assumes the view frustum is horizontally aligned
intp12 PlaneScanlineIntersection(intp12 cz, int32_t scanline)
{
	return cz*ScreenHeight/(tgY*(2*scanline-ScreenHeight));
}

// The output matrix is sparse, and only the following is stored
//
// invP = | a00 a02 |
//        | a11 a12 |
// a01 and a10 are implicity 0.
//
// x_world = invP * x_screen
void FillInvProjMatrix(intp12 cx, intp12 cy, intp12 cz, intp12 depth, Mat22p12& dst)
{
	dst.m[0][0] = TexelsPerMeter*2*depth*tgX/ScreenWidth;
	dst.m[0][1] = (cx-depth*tgX)*TexelsPerMeter + TexelOffset;
	// tgy/VRes = tgx/HRes, so we can reuse this from the first row.
	// Also nice because tgX has exact .8 representations more often
	dst.m[1][0] = depth*TexelsPerMeter;
	dst.m[1][1] = cy*TexelsPerMeter + TexelOffset;
}

int main()
{
	Display().StartBlank();
	Display().InitMode2();

	// TextInit
	TextSystem text;
	text.Init();

	// Init the frame counter
	auto* obj0 = &Sprite::OAM()[0].objects[0];
	obj0->attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
	obj0->attribute[1] = 0; // Left of the screen, small size
	obj0->attribute[2] = Sprite::DTile::HighSpriteBankIndex('0'-32);
	auto* obj1 = &Sprite::OAM()[0].objects[1];
	obj1->attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
	obj1->attribute[1] = 8; // Left of the screen, small size
	obj1->attribute[2] = Sprite::DTile::HighSpriteBankIndex('1'-32);

	// Set up HBlank interrupt
	REG_ISR_MAIN = hblank_cb;
	IO::DISPSTAT::Get().setBit<4>(); // Request Display to fire H-Blank interrupt
	IO::IE::Get().value = (1<<1); // Enable HBlank interrupts
	IO::IME::Get().value = 1;

	Display().enableSprites();
	Display().EndBlank();

	Vec3p12 camPos = {};
	camPos.z() = 1_p12;

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to ~1/16th of a millisecond
	while(1)
	{
		constexpr intp12 speed = 1_p12/60;
		// Logic
		if(Keypad::Held(Keypad::KEY_LEFT))
		{
			camPos.x() -= speed;
		}
		if(Keypad::Held(Keypad::KEY_RIGHT))
		{
			camPos.x() += speed;
		}
		if(Keypad::Held(Keypad::KEY_UP))
		{
			camPos.y() -= speed;
		}
		if(Keypad::Held(Keypad::KEY_DOWN))
		{
			camPos.y() += speed;
		}
		if(Keypad::Held(Keypad::KEY_L))
		{
			camPos.z() += 0.125_p12;
		}
		if(Keypad::Held(Keypad::KEY_R))
		{
			camPos.z() -= 0.125_p12;
		}

		// VSync
		Display().vSync();

		RefreshAffineTransforms(camPos);
		
		// Set the first transform for next frame
		auto& tx = gScanlineTransforms[0];

		// u,v offsets
		IO::BG2P::Get().refPoint.x() = tx.m[0][1].cast_down<8>();
		IO::BG2P::Get().refPoint.y() = tx.m[1][1].cast_down<8>();
		// Scale/rotation
		IO::BG2P::Get().A = int16_t(tx.m[0][0].cast_down<8>().raw);
		IO::BG2P::Get().B = 0;
		IO::BG2P::Get().C = 0;
		IO::BG2P::Get().D = int16_t(tx.m[1][0].cast_down<8>().raw);

		plotFrameIndicator();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond

		++t;
	}
	return 0;
}