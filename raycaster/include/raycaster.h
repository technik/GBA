#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>

extern "C"
 {
#include <tonc.h>
}

// IWRAM render functions
void verLine(uint16_t* backBuffer, int x, int drawStart, int drawEnd, int worldColor);