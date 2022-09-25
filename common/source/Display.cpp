#include <Display.h>

#ifdef _WIN32



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
    if (vertexShader == uint32_t(-1))
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

void InitBuffers(uint32_t& backBufferTexture)
{
    glGenTextures(1, &backBufferTexture);
    glBindTexture(GL_TEXTURE_2D, backBufferTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

#endif



bool Mode5Display::Init()
{
#ifdef _WIN32
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window with graphics context
    s_window = glfwCreateWindow(240 * 4, 160 * 4, "Mercury", NULL, NULL);
    if (s_window == NULL)
        return false;
    glfwMakeContextCurrent(s_window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(s_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Copy the vertex data of our full screen triangle
    glGenBuffers(1, &m_VBO);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fullScreenVertices), fullScreenVertices, GL_STATIC_DRAW);
    // 3. then set our vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    if (!LoadShader(m_fullScreenShader))
    {
        return false;
    }

    // Allocate the back buffer
    s_backBuffer.resize(Width * Height);
    InitBuffers(m_backBufferTexture);

    return true;
#else
    auto& disp = DisplayControl::Get();
    disp.SetMode<5, DisplayControl::BG2>();

    disp.BG2RotScale().a = (Width << 8) / ScreenWidth; // =(160/240.0)<<8
    disp.BG2RotScale().d = (Height << 8) / ScreenHeight; // =(128/160.0)<<8

    return true;
#endif
}

bool Mode5Display::BeginFrame()
{
#ifdef GBA
    return true;
#else
    if (glfwWindowShouldClose(s_window))
    {
        return false;
    }

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
    glfwGetFramebufferSize(s_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    return true;
#endif
}

void Mode5Display::Flip()
{
#ifdef GBA
    DisplayControl::Get().flipFrame();
#else
    // Copy our back buffer to the GPU
    glBindTexture(GL_TEXTURE_2D, m_backBufferTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, Width, Height, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, s_backBuffer.data());
    
    // Render a full screen triangle that samples from it
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_fullScreenShader);
    glBindVertexArray(m_VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_backBufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Finish the frame
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(s_window);
#endif
}