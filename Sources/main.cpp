#include "logo.hpp"

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#if BX_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

#include <iostream>

static bool s_showStats = false;

static void glfw_errorCallback(int error, const char* description)
{
    std::cerr << "GLFW error " << error << ": " << description << '\n';
}

static void glfw_keyCallback([[maybe_unused]] GLFWwindow* window, int key,
                             [[maybe_unused]] int scancode, int action,
                             [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_F1 && action == GLFW_RELEASE)
    {
        s_showStats = !s_showStats;
    }
}

int main()
{
    // Create a GLFW window without an OpenGL context.
    glfwSetErrorCallback(glfw_errorCallback);

    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window =
        glfwCreateWindow(1024, 768, "helloworld", nullptr, nullptr);
    if (!window)
    {
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(window, glfw_keyCallback);

    // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create
    // a render thread. Most graphics APIs must be used on the same thread that
    // created the window.
    bgfx::renderFrame();

    // Initialize bgfx using the native window handle and window resolution.
    bgfx::Init init;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    init.platformData.ndt = glfwGetX11Display();
    init.platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(window);
#elif BX_PLATFORM_OSX
    init.platformData.nwh = glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
    init.platformData.nwh = glfwGetWin32Window(window);
#endif

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    init.resolution.width = (uint32_t)width;
    init.resolution.height = (uint32_t)height;
    init.resolution.reset = BGFX_RESET_VSYNC;

    if (!bgfx::init(init))
    {
        return EXIT_FAILURE;
    }

    // Set view 0 to the same dimensions as the window and to clear the color
    // buffer.
    const bgfx::ViewId clearViewID = 0;
    bgfx::setViewClear(clearViewID, BGFX_CLEAR_COLOR);
    bgfx::setViewRect(clearViewID, 0, 0, bgfx::BackbufferRatio::Equal);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Handle window resize.
        int oldWidth = width, oldHeight = height;
        glfwGetWindowSize(window, &width, &height);
        if (width != oldWidth || height != oldHeight)
        {
            bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
            bgfx::setViewRect(clearViewID, 0, 0, bgfx::BackbufferRatio::Equal);
        }

        // This dummy draw call is here to make sure that view 0 is cleared if
        // no other draw calls are submitted to view 0.
        bgfx::touch(clearViewID);

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextImage(bx::max<uint16_t>(uint16_t(width / 2 / 8), 20) - 20,
                           bx::max<uint16_t>(uint16_t(height / 2 / 16), 6) - 6,
                           40, 12, s_logo, 160);
        bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats.");
        bgfx::dbgTextPrintf(0, 1, 0x0f,
                            "Color can be changed with ANSI "
                            "\x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;"
                            "mp\x1b[14;me\x1b[0m code too.");
        bgfx::dbgTextPrintf(
            80, 1, 0x0f,
            "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    "
            "\x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
        bgfx::dbgTextPrintf(
            80, 2, 0x0f,
            "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    "
            "\x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

        const bgfx::Stats* stats = bgfx::getStats();
        bgfx::dbgTextPrintf(0, 2, 0x0f,
                            "Backbuffer %dW x %dH in pixels, debug text %dW x "
                            "%dH in characters.",
                            stats->width, stats->height, stats->textWidth,
                            stats->textHeight);

        // Enable stats or debug text.
        bgfx::setDebug(s_showStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);

        // Advance to next frame. Process submitted rendering primitives.
        bgfx::frame();
    }

    bgfx::shutdown();
    glfwTerminate();

    return EXIT_SUCCESS;
}