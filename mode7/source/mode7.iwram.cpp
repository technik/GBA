//
// mode7.iwram.c
// Interrupts
//
extern "C" {
	#include <tonc.h>
}

#include <demo.h>

void m7_hbl_c()
{
	if(REG_VCOUNT >= 160 | REG_VCOUNT < 79)
		return;
	setBg2AffineTx(REG_VCOUNT+1);
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
	FIXED lambda = (gCamPos.z * lu_div(vCount-scanlineOffset))/(1<<12); // .8*.16 /.12 = .12
	FIXED lcf= (lambda*gCosf)/(1<<5); // .12*.8 /.8 = .12
	FIXED lsf= (lambda*gSinf)/(1<<5); // .12*.8 /.8 = .12

	REG_BG2PA = (lcf+(1<<3))/(1<<4); // .12/.4=.8
	REG_BG2PC = (lsf+(1<<3))/(1<<4); // .12/.4=.8

	REG_BG2X = gCamPos.x - (lcf*120 - lsf*160 + (1<<3))/(1<<4);
	REG_BG2Y = gCamPos.y - (lsf*120 + lcf*160 + (1<<3))/(1<<4);
}