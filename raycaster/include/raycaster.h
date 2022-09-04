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

class Mode4Renderer
{
public:
    static void Init();
    static void DrawMinimap(uint16_t* backBuffer, math::Vec3p8 centerPos);
    static void RenderWorld(const Camera& cam);
private:
    static void yDLine(uint16_t* backBuffer, unsigned x, unsigned drawStart, unsigned drawEnd, uint16_t worldColor);
};

// IWRAM render functions
void RenderMode3(const Camera& cam);