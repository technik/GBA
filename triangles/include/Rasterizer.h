#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>
#include <Camera.h>
#include <matrix.h>
#include <vector.h>
#include <Color.h>
#include <WAD.h>

#ifdef GBA
extern "C" {
#include <tonc.h>
}
#endif // GBA

class Rasterizer
{
public:
    using DisplayMode = Mode5Display;
    static constexpr int32_t ScreenWidth = DisplayMode::Width;
    static constexpr int32_t ScreenHeight = DisplayMode::Height;

    static void Init();
    static void RenderWorld(const Camera& cam, const math::Mat44p16& projMtx);
    static bool BeginFrame();
    static void EndFrame();

    static void DrawLine(uint16_t* buffer, int stride, int16_t color, math::Vec2p16 a, math::Vec2p16 b, int xEnd, int yEnd);
    static void DrawHorizontalLine(uint16_t* buffer, int stride, int16_t color, int row, int xStart, int xEnd);
    static void DrawVerticalLine(uint16_t* buffer, int stride, int16_t color, int col, int y0, int y1);

    static inline Color skyClr = BasicColor::SkyBlue;
    static inline Color groundClr = BasicColor::DarkGrey;

private:
    inline static DisplayMode displayMode;
};