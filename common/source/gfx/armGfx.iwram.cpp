// GFX code that needs to live in IWRAM
#ifdef GBA
extern "C" {
	#include <tonc.h>
}
#endif // GBA

#include <gfx/palette.h>
#include <Device.h>
#include <Color.h>
#include <gfx/palette.h>
#include <Display.h>
#include <gfx/armGfx.h>

using namespace math;

namespace // Exposed shared state
{
	math::Vec3p8 gCamPos;
	math::intp8 gCosf;
	math::intp8 gSinf;
}

void PostCameraState(
	math::Vec3p8 Pos,
	math::intp8 Cosf,
	math::intp8 Sinf)
{
	gCamPos = Pos;
	gCosf = Cosf;
	gSinf = Sinf;
}

void m7_hbl_c()
{
	auto vCount = IO::VCOUNT::Value();
	if(vCount >= 160 || vCount < 80)
		return;

	setBg2AffineTx(vCount+1);
}

void setBg2AffineTx(uint16_t vCount)
{
#ifdef GBA
	constexpr int32_t scanlineOffset = ScreenHeight/2;
	//constexpr int32_t scanlineRange = ScreenHeight-scanlineOffset;
	// Using tg(y) = 0.5, vertical field of view ~= 53.13 deg.
	// d = (z * VRes/2) / ((vCount-VRes/2)*tgy)
	// d = (z * VRes/2) / ((vCount-VRes/2)*0.5)
	// d = (z * VRes) / (vCount-VRes/2)
	// Lambda = d*2*tgy/VRes
	// Lambda = z/(vCount-VRes/2)

	// No point rounding this up because the table only takes integers, and div by zero already gives the largest possible integer
	// when multiplied by z (safe for maybe the last 11 bits).
	// By adding (1<<11) while rounding, it seems we overflow the integer before down casting and bounce back to 0 again,
	// meaning we see a line in the horizon.
	intp16 div = intp16::castFromShiftedInteger<16>(lu_div(vCount-scanlineOffset));
	auto lambda = (gCamPos.z() * div).cast<12>();

	// d = Lambda*VRes/(2)
	// dx = d*tgx = lambda*VRes/(2*tgy) = lambda * VRes
	constexpr int32_t kTexelsPerMeter = 8;

	auto lcf = (lambda*gCosf).cast<12>() * kTexelsPerMeter;
	auto lsf = (lambda*gSinf).cast<12>() * kTexelsPerMeter;

	IO::BG2P::Get().A = lcf.cast<8>().raw;
	IO::BG2P::Get().C = -lsf.cast<8>().raw;
	
	REG_BG2X = (gCamPos.x()*kTexelsPerMeter - (lcf*120 + lsf*160).cast<8>()).raw;
	REG_BG2Y = 512 * 256 + (- gCamPos.y()*kTexelsPerMeter + (lsf*120 - lcf*160).cast<8>()).raw;
#endif // GBA
}
