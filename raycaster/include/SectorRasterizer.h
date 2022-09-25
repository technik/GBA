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
static constexpr size_t LevelDataSize = sizeof(WAD::LevelData);

bool loadWAD(WAD::LevelData& dstLevel);


class SectorRasterizer
{
public:
    using DisplayMode = Mode5Display;

    static void Init();
    static void RenderWorld(WAD::LevelData& level, const Camera& cam);
    static bool BeginFrame();
    static void EndFrame();

    static inline uint16_t skyClr = BasicColor::SkyBlue.raw;
    static inline uint16_t groundClr = BasicColor::DarkGreen.raw;

private:
    inline static DisplayMode displayMode;

    struct DepthBuffer
    {
        uint8_t lowBound[DisplayMode::Width];
        uint8_t highBound[DisplayMode::Width];

        void Clear();
    };

    static void RenderSubsector(const WAD::LevelData& level, uint16_t ssIndex, const Camera& cam, DepthBuffer& depthBuffer);
    static void RenderBSPNode(const WAD::LevelData& level, uint16_t nodeIndex, const Camera& cam, DepthBuffer& depthBuffer);
    static void RenderWall(const Camera& cam, const math::Vec2p8& A, const math::Vec2p8& B, Color clr, DepthBuffer& depthBuffer);
};