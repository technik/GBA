// Implement a ray tracer that can be used both on PC or in the GBA
// Uses fixed point math, dithering and color palettes for compatibility
// with the GBA, but can easily be debugged on PC.
#include <linearMath.h>
#include <Camera.h>
#include <Display.h>
#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <imageUtils.h>

using namespace math;

int main()
{
    // Create a render target image
    Image16bit renderTarget;
    renderTarget.resize(ScreenWidth, ScreenHeight);
 
    // Load/Generate a map
    RawImage mapImage;
    mapImage.load("heightmap.png");

    // Create a camera
    Camera cam(ScreenWidth, ScreenHeight, Vec3p8(0_p8, 0_p8, 0_p8));
    // Render
    // Save the image
    return 0;
}