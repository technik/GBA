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

void m7_hbl_c()
{
	auto vCount = IO::VCOUNT::Value();
	if(vCount >= 160 | vCount < 79)
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
	auto lambda = math::Fixed<int32_t,12>::castFromShiftedInteger<24>(gCamPos.z * lu_div(vCount-scanlineOffset));
	auto lcf = (lambda*gCosf).cast<12>() * 8;
	auto lsf = (lambda*gSinf).cast<12>() * 8;

	IO::BG2P::Get().A = lcf.cast<8>().raw;
	IO::BG2P::Get().C = lsf.cast<8>().raw;

	REG_BG2X = gCamPos.x - (lcf*120 - lsf*160).cast<8>().raw;
	REG_BG2Y = gCamPos.y - (lsf*120 + lcf*160).cast<8>().raw;
}
