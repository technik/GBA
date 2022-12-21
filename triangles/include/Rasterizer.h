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

// Draws a line in octant 0 or 3 ( |DeltaX| >= DeltaY )
// Snaps start and end points to the center of their pixels
// Op syntax: void Op(int x, int y) const
// Omits the line's first pixel
template<class Op>
void RasterLineOctant0(const Op& op, int x, int y, int dx, int dy, int signX)
{
    int dy2 = 2 * dy;
    int yStep = dy2 - 2 * dx;
    int runningError = dy2 - dx;
    
    while (dx--)
    {
        if (runningError >= 0)
        {
            ++y;
            runningError += yStep;
        }
        else
        {
            runningError += dy2;
        }
        x += signX;
        Op(x, y);
    }
}

// Draws a line in octant 1 or 2 ( |DeltaX| < DeltaY )
// Op syntax: void Op(int x, int y) const
// Omits the line's first pixel
template<class Op>
void RasterLineOctant1(const Op& op, int x, int y, int dx, int dy, int signX)
{
    int dx2 = 2 * dx;
    int xStep = dx2 - 2 * dy;
    int runningError = dx2 - dy;

    while (dy--)
    {
        if (runningError >= 0)
        {
            x += signX;
            runningError += xStep;
        }
        else
        {
            runningError += dx2;
        }
        y++;
        Op(x, y);
    }
}

// Op syntax: void Op(int x, int y) const
template<class Op>
void rasterLine(const Op& op, const math::Vec2p16& a, const math::Vec2p16& b)
{
    // Force drawing lines in one of the first 4 octants
    if (a.y > b.y)
    {
        std::swap(a, b);
    }

    int x0 = a.x.floor();
    int y0 = a.y.floor();
    int dx = b.x.floor() - x0;
    int dy = b.y.floor() - y0;

    Op(x0, y0); // Draw the line's first pixel

    // Choose the correct octant and deltas
    if (dx > 0)
    {
        if (dx >= dy)
        {
            RasterLineOctant0(op, x0, y0, dx, dy, 1);
        }
        else
        {
            RasterLineOctant1(op, x0, y0, dx, dy, 1);
        }
    }
    else
    {
        if (-dx >= dy)
        {
            RasterLineOctant0(op, x0, y0, -dx, dy, -1);
        }
        else
        {
            RasterLineOctant1(op, x0, y0, -dx, dy, -1);
        }
    }
}

class Rasterizer
{
public:
    using DisplayMode = Mode5Display;
    static constexpr int32_t ScreenWidth = DisplayMode::Width;
    static constexpr int32_t ScreenHeight = DisplayMode::Height;

    static void Init();
    static void RenderWorld(const YawPitchCamera& cam);
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