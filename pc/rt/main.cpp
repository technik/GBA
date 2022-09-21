// Implement a ray tracer that can be used both on PC or in the GBA
// Uses fixed point math, dithering and color palettes for compatibility
// with the GBA, but can easily be debugged on PC.

#include <linearMath.h>
#include <Camera.h>
#include <Display.h>
#include <cassert>
#include <imageUtils.h>

#include <../../raycaster/include/SectorRasterizer.h>

// Embedded assets
#include <mercury.wad.h>
#include <test.wad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace math;

int main()
{
    Mode5Display displayMode; 
    displayMode.Init();

    // Load a WAD map
    WAD::LevelData level;
    loadWAD(level);
    //loadWAD(mercury_WAD);

    // Create a camera
    Camera cam(Mode5Display::Width, Mode5Display::Height, Vec3p8(0_p8, 0_p8, 0_p8));

    // Debug test pose
    cam.m_pose.phi.raw = 0;// 39300;
    cam.m_pose.pos.m_x.raw = -48;
    cam.m_pose.pos.m_y.raw = -96;
    cam.m_pose.update();

    // Render
    SectorRasterizer::Init();

    SectorRasterizer::RenderWorld(level, cam);
    
    // Save the image
    Mode5Display::s_backBuffer.save("render.png");
    return 0;
}