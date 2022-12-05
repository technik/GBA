#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>
#include <Camera.h>
#include <vector.h>

#ifdef GBA
extern "C" {
#include <tonc.h>
}
#endif // GBA

static constexpr int kMapRows = 16;
static constexpr int kMapCols = 16;

extern uint8_t g_worldMap[kMapRows * kMapCols];

class Mode4Renderer
{
public:
    static void Init();
    static void RenderWorld(const Camera& cam);
private:
    static void yDLine(uint16_t* backBuffer, unsigned x, unsigned drawStart, unsigned drawEnd, uint16_t worldColor);
    static inline uint32_t sPaletteStart;
};

// IWRAM render functions
void RenderMode3(const Camera& cam);