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

class SectorRasterizer
{
public:
    static void Init();
    static void RenderWorld(const Camera& cam);

    static inline uint16_t fillClr = BasicColor::SkyBlue.raw;
    static inline uint16_t fillClr2 = BasicColor::Red.raw;

private:
    static void RenderWall();

    using DisplayMode = Mode5Display;
};