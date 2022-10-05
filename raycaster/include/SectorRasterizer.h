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

    // Render structures
    struct VisPlane
    {
        static constexpr uint32_t kMaxWidth = DisplayMode::Width;
        // TODO: In theory, visplanes can't span more than half a screen height, so maybe the limit should be 512?
        static_assert(DisplayMode::Width < 256, "VisPlane heights may not fit in a byte");
        math::intp16 height;
        math::intp16 lightLevel;
        int32_t textureNdx;
        int16_t minX;
        int16_t maxX;
        int32_t padding;
        uint8_t top[kMaxWidth];
        uint8_t bottom[kMaxWidth];
    };

    static void Merge(VisPlane& dst, const VisPlane& src);
    static bool CanMerge(const VisPlane& a, const VisPlane& b);

    static void Init();
    static void RenderWorld(WAD::LevelData& level, const Camera& cam);
    static bool BeginFrame();
    static void EndFrame();

    static inline Color skyClr = BasicColor::SkyBlue;
    static inline Color groundClr = BasicColor::DarkGrey;

private:
    inline static DisplayMode displayMode;

    struct DepthBuffer
    {
        uint8_t floorClip[DisplayMode::Width];
        uint8_t ceilingClip[DisplayMode::Width];

        void Clear();
    };

    static void RenderSubsector(const WAD::LevelData& level, uint16_t ssIndex, const Pose& view, DepthBuffer& depthBuffer);
    static void RenderBSPNode(const WAD::LevelData& level, uint16_t nodeIndex, const Pose& view, DepthBuffer& depthBuffer);
    static void RenderWall(
        const math::Vec2p16& ndcA, const math::Vec2p16& ndcB,
        const math::intp16& floorH, const math::intp16& ceilingH,
        Color ceilColr, Color gndClr, const math::intp16& lightLevel,
        DepthBuffer& depthBuffer);
    static void RenderPortal(const Pose& view,
        const math::Vec2p16& ndcA, const math::Vec2p16& ndcB,
        const math::intp16& floorH, const math::intp16& ceilingH,
        const WAD::Sector& backSector,
        Color ceilColr, Color gndClr, Color clr, DepthBuffer& depthBuffer);
};