// Implement a ray tracer that can be used both on PC or in the GBA
// Uses fixed point math, dithering and color palettes for compatibility
// with the GBA, but can easily be debugged on PC.
#include <linearMath.h>
#include <Camera.h>
#include <Display.h>
#include <cassert>
#include <imageUtils.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


using namespace math;

void renderVoxelTerrain(const RawImage& map, const Camera& cam, Image16bit& renderTarget)
{
    Color16b clearColor = Color3f{ 255, 255, 255 };
    // Vertical line scans
    for (int col = 0; col < renderTarget.width; ++col)
    {
        // Find 2d tracing direction
        // Start tracing at min distance
        int32_t h = renderTarget.height;
        // While height not filled
        // while(h && not far enough)
        // {
        //     // Sample height
        //     auto nextH = sample(map);
        //     while (h > nextH)
        //     {
        //         renderTarget.at(col, h--) = color(map);
        //     }
        // }

        while (h) // Not filled the column
        {
            renderTarget.at(col, h--) = clearColor;
        }
    }
}

int main()
{
    // Create a render target image
    Image16bit renderTarget;
    renderTarget.resize(ScreenWidth, ScreenHeight);
 
    // Load/Generate a map
    RawImage mapImage;
    mapImage.load("heightmap.png");

    // Create a camera
    Camera cam(ScreenWidth, ScreenHeight, Vec3p8(0_p8, intp8(mapImage.width/2), 0_p8));

    // Render
    
    // Save the image
    renderTarget.save("render.png");
    return 0;
}