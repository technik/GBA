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
	if(REG_VCOUNT >= 160)
		return;
	setBg2AffineTx(REG_VCOUNT+1);
}

void setBg2AffineTx(uint16_t vCount)
{
	FIXED lambda = cam_pos.y*lu_div(vCount)>>12;	// .8*.16 /.12 = 20.12
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
	REG_BG2Y= cam_pos.z - lxr - lyr;
}
