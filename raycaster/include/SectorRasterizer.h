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

class Mode3SectorRasterizer
{
public:
    static void Init();
    static void RenderWorld(const Camera& cam);

private:
    static void RenderWall();
};