// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>

#include <glad.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

float fullScreenVertices[] = {
    // x,    y,    z,   u,   v
    -1.0f, -1.0f, 0.0f, 0.f, 1.f,
     3.0f, -1.0f, 0.0f, 2.0f, 1.f,
    -1.0f,  3.0f, 0.0f, 0.f, -1.f,
};

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

uint32_t LoadSubShader(const char* fileName, uint32_t shaderType)
{
    // Read vertex shader file
    std::ifstream shaderFile(fileName, std::ios_base::ate);
    auto fileSize = shaderFile.tellg();
    shaderFile.seekg(std::ios_base::beg);
    std::vector<char> code(fileSize);
    shaderFile.read(code.data(), fileSize);

    uint32_t shader;
    shader = glCreateShader(shaderType);

    const char* rawCode = code.data();
    glShaderSource(shader, 1, &rawCode, NULL);
    glCompileShader(shader);

    // Check for errors
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader compilation failed for file:" << fileName << "\n" << infoLog << std::endl;

        return uint32_t(-1);
    }

    return shader;
}

bool LoadShader(uint32_t& shaderProgram)
{
    // Read vertex shader code
    auto vertexShader = LoadSubShader("fullScreen.vtx", GL_VERTEX_SHADER);
    if(vertexShader == uint32_t(-1))
    {
        return false;
    }

    // Read pixel shader
    auto fragmentShader = LoadSubShader("fullScreen.pxl", GL_FRAGMENT_SHADER);
    if (fragmentShader == uint32_t(-1))
    {
        glDeleteShader(vertexShader);
        return false;
    }

    // Link into a full shader
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Check for errors
    int  success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        
        return false;
    }

    return true;
}

class DeviceMode
{
public:
    bool Init()
    {
        // Copy the vertex data of our full screen triangle
        glGenBuffers(1, &m_VBO);

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        // 2. copy our vertices array in a buffer for OpenGL to use
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fullScreenVertices), fullScreenVertices, GL_STATIC_DRAW);
        // 3. then set our vertex attributes pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        if (!LoadShader(m_fullScreenShader))
        {
            return false;
        }

        InitBuffers();

        return true;
    }

    void drawFrame()
    {
        // Generate an image with the software rasterizer
        // Copy it to the GPU
        // Render a full screen triangle that samples from it
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_fullScreenShader);
        glBindVertexArray(m_VAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_backBufferTexture);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

private:
    void InitBuffers()
    {
        backBuffer.resize(160 * 120 * 3);

        // Fill texture with data
        for (int i = 0; i < 120; ++i)
        {
            for (int j = 0; j < 160; ++j)
            {
                backBuffer[3 * (i * 160 + j) + 0] = i^ j;
                backBuffer[3 * (i * 160 + j) + 1] = i^ j;
                backBuffer[3 * (i * 160 + j) + 2] = i^ j;
            }
        }

        glGenTextures(1, &m_backBufferTexture);
        glBindTexture(GL_TEXTURE_2D, m_backBufferTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 160, 120, 0, GL_RGB, GL_UNSIGNED_BYTE, backBuffer.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    unsigned int m_backBufferTexture;
    std::vector<uint8_t> backBuffer;

    unsigned int m_VBO;
    unsigned int m_VAO;
    uint32_t m_fullScreenShader = uint32_t(-1);
};

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(240*4, 160*4, "Mercury", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    DeviceMode gbaDevice;

    bool loadOk = gbaDevice.Init();

    // Main loop
    if (loadOk)
    {
        while (!glfwWindowShouldClose(window))
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            gbaDevice.drawFrame();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
