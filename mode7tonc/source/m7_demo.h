#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#ifdef __cplusplus
extern "C" {
#endif

#include <tonc.h>

// === CONSTANTS & MACROS =============================================

#define M7_D 160

// === GLOBALS ========================================================

extern VECTOR cam_pos;
extern u16 cam_phi;
extern FIXED g_cosf, g_sinf;

// === PROTOTYPES =====================================================
IWRAM_CODE void m7_hbl_c();
IWRAM_CODE void setBg2AffineTx(uint16_t vCount);

#ifdef __cplusplus
} // extern C
#endif