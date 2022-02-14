#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>

extern "C" {

#include <tonc.h>

// === CONSTANTS & MACROS =============================================

#define M7_D 160

// === GLOBALS ========================================================
extern VECTOR gCamPos;
extern FIXED gCosf;
extern FIXED gSinf;

// === PROTOTYPES =====================================================
IWRAM_CODE void m7_hbl_c();
IWRAM_CODE void setBg2AffineTx(uint16_t vCount);

} // extern C