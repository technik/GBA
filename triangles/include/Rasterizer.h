#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>
#include <Camera.h>
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
    static void RenderWorld(const Camera& cam);
    static bool BeginFrame();
    static void EndFrame();

    static inline Color skyClr = BasicColor::SkyBlue;
    static inline Color groundClr = BasicColor::DarkGrey;

private:
    inline static DisplayMode displayMode;
};