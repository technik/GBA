// Test the fixed point math library we use on the GBA
#include <linearMath.h>
#include <Camera.h>
#include <cassert>

using namespace math;

void testCamera()
{
    auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(0_p8, 0_p8, 0_p8));

    Vec3p8 objPos = camera.m_pose.pos;

    auto ss = camera.projectWorldPos(objPos);
    assert(ss.z().roundToInt() == 0); // X and Y are undefined in this case, so we don't test it

    objPos.y() += 10_p8;
    ss = camera.projectWorldPos(objPos);
    assert(ss.z().roundToInt() == 10);
    // x and y centered in the screen
    assert(ss.x().roundToInt() == ScreenWidth / 2);
    assert(ss.y().roundToInt() == ScreenHeight / 2);

    // Move the camera towards the object
    camera.m_pose.pos.y() += 5_p8;
    ss = camera.projectWorldPos(objPos);
    assert(ss.z().roundToInt() == 5);

    objPos = camera.m_pose.pos;
    objPos.y() -= 10_p8;
    ss = camera.projectWorldPos(objPos);
    assert(ss.z().roundToInt() == -10);
    // x and y centered in the screen
    assert(ss.x().roundToInt() == ScreenWidth / 2);
    assert(ss.y().roundToInt() == ScreenHeight / 2);

    // Test off center coordinates
    objPos = camera.m_pose.pos;
    objPos.y() += 10_p8;
    objPos.x() += 7.5_p8; // Right edge of the screen
    objPos.z() += 5_p8; // Upper edge of the screen

    ss = camera.projectWorldPos(objPos);
    assert(ss.z().roundToInt() == 10);
    // x and y centered in the screen
    auto roundedX = ss.x().roundToInt();
    assert(roundedX >= ScreenWidth-1 && roundedX <= ScreenWidth+1);
    assert(ss.y().roundToInt() == 0);

    objPos = camera.m_pose.pos;
    objPos.y() += 10_p8;
    objPos.x() -= 7.5_p8; // Left edge of the screen
    objPos.z() -= 5_p8; // Lower edge of the screen

    ss = camera.projectWorldPos(objPos);
    assert(ss.z().roundToInt() == 10);
    // x and y centered in the screen
    roundedX = ss.x().roundToInt();
    auto roundedY = ss.y().roundToInt();
    assert(roundedX >= -1 && roundedX <= 1);
    assert(roundedY == ScreenHeight);

    // Rotate the game 90 degrees
    camera.m_pose.phi = intp8(0.25f);
    camera.m_pose.update();
    ss = camera.projectWorldPos(objPos);
    assert(camera.m_pose.sinf == 1_p8);
    assert(camera.m_pose.cosf == 0_p8);

    objPos = camera.m_pose.pos;
    objPos.x() -= 10_p8;
    objPos.y() -= 7.5_p8; // Left edge of the screen
    objPos.z() -= 5_p8; // Lower edge of the screen
    ss = camera.projectWorldPos(objPos);

    roundedX = ss.x().roundToInt();
    roundedY = ss.y().roundToInt();
    assert(roundedX >= -1 && roundedX <= 1);
    assert(roundedY == ScreenHeight);
}

int main()
{
    testCamera();

    return 0;
}