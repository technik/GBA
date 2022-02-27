//
// mode7.iwram.c
// Interrupts
//
extern "C" {
	#include <tonc.h>
}
#include <Device.h>
#include <Color.h>
#include <gfx/palette.h>
#include <demo.h>

using namespace math;

void m7_hbl_c()
{
	auto vCount = IO::VCOUNT::Value();
	if(vCount >= 160 | vCount < 90)
		return;

	setBg2AffineTx(vCount+1);
}

void setBg2AffineTx(uint16_t vCount)
{
	constexpr int32_t scanlineOffset = ScreenHeight/2;
	constexpr int32_t scanlineRange = ScreenHeight-scanlineOffset;
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
	intp16 div; div.raw = lu_div(vCount-scanlineOffset);
	auto lambda = (gCamPos.z() * div).cast<12>();
	auto lcf = (lambda*gCosf).cast<12>() * 8;
	auto lsf = (lambda*gSinf).cast<12>() * 8;

	IO::BG2P::Get().A = lcf.cast<8>().raw;
	IO::BG2P::Get().C = -lsf.cast<8>().raw;

	REG_BG2X = (gCamPos.x() - (lcf*120 + lsf*160).cast<8>()).raw;
	REG_BG2Y = (gCamPos.y() - (lcf*160 - lsf*120).cast<8>()).raw;
}
