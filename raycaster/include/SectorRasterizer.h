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

// Util
static constexpr size_t LevelDataSize = sizeof(LevelData);

IWRAM_CODE bool loadWAD(LevelData& dstLevel, const uint32_t* wadData);

class SectorRasterizer
{
public:
    using DisplayMode = Mode5Display;

    static void Init();
    static void RenderWorld(LevelData& level, const Camera& cam);

    static inline uint16_t fillClr = BasicColor::SkyBlue.raw;
    static inline uint16_t fillClr2 = BasicColor::Red.raw;

private:
    static void RenderSubsector(const LevelData& level, uint16_t ssIndex, const Camera& cam);
    static void RenderBSPNode(const LevelData& level, uint16_t nodeIndex, const Camera& cam);
    static void RenderWall(const Camera& cam, const math::Vec2p8& A, const math::Vec2p8& B, Color clr);
};