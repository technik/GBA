//
// m7_isrs.iwram.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.

#include <tonc.h>

#include "m7_demo.h"

// Perspective zoom for this scanline
// Note that this is actually wrong, because 
// the hblank occurs AFTER REG_VCOUNT is drawn, screwing up 
// at scanline 0. Oh well.

///////////////////////////////////////////////////////////////////////

// --- Type C ---
//'proper' mode 7, with mixed fixed-point. Smooth
// - offset * (zoom * rotate)
// - lambda is .12f for xs and ys
// - 120 multiplied before shift in xs
// * .12 lambda,lcf,lsf would work too.
void m7_hbl_c()
{
	if(REG_VCOUNT >= 160 | REG_VCOUNT < 80)
		return;
	setBg2AffineTx(REG_VCOUNT+1);
}

#define NEW_PROJ_MATH

void setBg2AffineTx(uint16_t vCount)
{
#ifdef NEW_PROJ_MATH
	constexpr int32_t scanlineOffset = ScreenHeight/2;
	constexpr int32_t scanlineRange = ScreenHeight-scanlineOffset;
	// Using tg(y) = 0.5, vertical field of view ~= 53.13 deg.
	// d = (z * VRes/2) / ((vCount-VRes/2)*tgy)
	// d = (z * VRes/2) / ((vCount-VRes/2)*0.5)
	// d = (z * VRes) / (vCount-VRes/2)
	// Lambda = d*2*tgy/VRes
	// Lambda = z/(vCount-VRes/2)
	FIXED lambda = (cam_pos.z * lu_div(vCount-scanlineOffset))/(1<<12); // .8*.16 /.12 = .12
	FIXED lcf= (lambda*g_cosf)/(1<<8); // .12*.8 /.8 = .12
	FIXED lsf= (lambda*g_sinf)/(1<<8); // .12*.8 /.8 = .12

	REG_BG2PA = (lcf+(1<<3))/(1<<4); // .12/.4=.8
	REG_BG2PC = (lsf+(1<<3))/(1<<4); // .12/.4=.8

	REG_BG2X = cam_pos.x - (lcf*120 - lsf*160)/(1<<4);
	REG_BG2Y = cam_pos.y - (lsf*120 + lcf*160)/(1<<4);
#else // NEW_PROJ_MATH
	FIXED lambda = cam_pos.z*lu_div(vCount)>>12;	// .8*.16 /.12 = 20.12
	FIXED lcf= lambda*g_cosf>>8;						// .12*.8 /.8 = .12
	FIXED lsf= lambda*g_sinf>>8;						// .12*.8 /.8 = .12
	
	REG_BG2PA= lcf>>4;
	REG_BG2PC= lsf>>4;

	// Offsets
	// Note that the lxr shifts down first! 

	// horizontal offset
	FIXED lxr= 120*(lcf>>4);
	FIXED lyr= (M7_D*lsf)>>4;
	REG_BG2X= cam_pos.x - lxr + lyr;

	// vertical offset
	lxr= 120*(lsf>>4);
	lyr= (M7_D*lcf)>>4;	
	REG_BG2Y= cam_pos.y - lxr - lyr;
#endif // NEW_PROJ_MATH
}
