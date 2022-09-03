#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>
#include <Camera.h>
#include <vector.h>

extern "C"
 {
#include <tonc.h>
}

static constexpr int kMapRows = 16;
static constexpr int kMapCols = 16;

// IWRAM render functions
void yDLine(uint16_t* backBuffer, unsigned x, unsigned drawStart, unsigned drawEnd, uint16_t worldColor);
void DrawMinimapMode4(uint16_t* backBuffer, math::Vec3p8 centerPos);
void RenderMode4(const Camera& cam);
void RenderMode3(const Camera& cam);