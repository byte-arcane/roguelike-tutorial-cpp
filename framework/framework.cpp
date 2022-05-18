#include "framework.h"

// C++
#include <string>
#include <iostream>
#include <filesystem>

// GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Dear ImGui
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stb_image.h>

#include "input.h"

// GLFW window related
GLFWwindow* glfWindow = NULL;

int rlf::FrameworkApp::viewportWidth = 0;
int rlf::FrameworkApp::viewportHeight = 0;

// Callback for GLFW related errors
static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "[ERROR] GLFW error: " << error << ", " << description << std::endl;
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    rlf::Input::KeyCallback(key, action);
}

static void glfw_mcur_callback(GLFWwindow* window, double xpos, double ypos)
{
    rlf::Input::MouseCursorCallback((float)xpos, (float)ypos);
}

static void glfw_mbtn_callback(GLFWwindow* window, int button, int action, int mods)
{
    rlf::Input::MouseButtonCallback(button, action);
}

// Taken from https://learnopengl.com/In-Practice/Debugging
// Set breakpoints in here and check the call stack to see what triggered this output
static void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

// Termination function -- release resources
void teardown()
{
    // GUI-related
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLFW-related
    if (glfWindow != NULL)
        glfwDestroyWindow(glfWindow);
    glfwTerminate();
}

// Initialisation function
bool initializeGLFW(const rlf::FrameworkApp::WindowSettings& settings)
{
    // Set callback to handle any GLFW errors
    glfwSetErrorCallback(glfw_error_callback);

    // Initialise GLFW library and create window
    if (!glfwInit())
    {
        std::cerr << "[ERROR] Couldn't initialize GLFW" << std::endl;
        return false;
    }
    else
    {
        std::cout << "[INFO] GLFW initialized" << std::endl;
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_SAMPLES, settings.samples);
    
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // allow debugging

    auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    const int marginSize = settings.fullscreen ? 0 : 50;
    
    glfWindow = glfwCreateWindow(
        mode->width - marginSize *2,
        mode->height - marginSize *2,
        "Framework",
        settings.fullscreen ? monitor : NULL,
        NULL
    );
    if (!glfWindow)
    {
        std::cerr << "[ERROR] Couldn't create a GLFW window" << std::endl;
        return false;
    }

    glfwSetWindowPos(glfWindow, marginSize, marginSize);

    // Set callback to handle any GLFW errors
    glfwSetKeyCallback(glfWindow, glfw_key_callback);
    glfwSetMouseButtonCallback(glfWindow, glfw_mbtn_callback);
    glfwSetCursorPosCallback(glfWindow, glfw_mcur_callback);

    glfwMakeContextCurrent(glfWindow);
    // VSync
    glfwSwapInterval(1);

    std::cout << "[INFO] OpenGL from GLFW "
        << glfwGetWindowAttrib(glfWindow, GLFW_CONTEXT_VERSION_MAJOR)
        << "."
        << glfwGetWindowAttrib(glfWindow, GLFW_CONTEXT_VERSION_MINOR)
        << std::endl;

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW initialization failed.\n";
        return false;
    }

    // Setup opengl debugging messages
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        // initialize debug output 
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    return true;
}

bool initializeDearImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // setup platform/renderer bindings
    if (!ImGui_ImplGlfw_InitForOpenGL(glfWindow, true)) { return false; }
    if (!ImGui_ImplOpenGL3_Init()) { return false; }

    return true;
}

namespace rlf
{
	int FrameworkApp::run()
	{
        // framework initialisation
        if (!initializeGLFW(settings))
        {
            std::cerr << "[ERROR] GLFW initialization failed" << std::endl;
            return EXIT_FAILURE;
        }

        // Get viewport information
        int viewportData[4];
        glGetIntegerv(GL_VIEWPORT, viewportData);
        viewportWidth = viewportData[2];
        viewportHeight = viewportData[3];

        if (!initializeDearImGui())
        {
            std::cerr << "[ERROR] Dear ImGui initialization failed" << std::endl;
            return EXIT_FAILURE;
        }

        // user-defined initialisation
        onInit();

        // rendering loop
        while (!glfwWindowShouldClose(glfWindow))
        {
            // user-defined update code
            onUpdate();

            // user-defined rendering code
            onRender();

            // GUI-related preamble
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("GUI", NULL, ImGuiWindowFlags_AlwaysAutoResize);

            // user-defined GUI code. This supports a single window, from the ::Begin() call above
            onGui();
            ImGui::End();

            // GUI rendering code
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // swap the display buffer with the one we just rendered to
            glfwSwapBuffers(glfWindow);

            // continuous rendering, even if window is not visible or minimized
            glfwPollEvents();
        }

        // user-defined termination code
        onTerminate();

        // framework termination code
        teardown();

        return 0;
	}

    void FrameworkApp::quit()
    {
        glfwSetWindowShouldClose(glfWindow, 1);
    }
}