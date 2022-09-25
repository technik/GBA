// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <Display.h>

class DeviceMode
{
public:
    bool Init()
    {
        

        return true;
    }

    void drawFrame()
    {
        
    }

private:

};

int mainDemo(int, char**)
{
    Mode5Display displayMode;
    if (!displayMode.Init())
    {
        return -1;
    }
    DeviceMode gbaDevice;

    bool loadOk = gbaDevice.Init();

    // Main loop
    if (loadOk)
    {
        while (1)
        {
            if (!displayMode.BeginFrame())
            {
                return 0;
            }
            
            gbaDevice.drawFrame();
        }
    }

    // Cleanup
    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();
    //
    //glfwDestroyWindow(window);
    //glfwTerminate();

    return 0;
}
