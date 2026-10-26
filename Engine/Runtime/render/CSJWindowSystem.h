#pragma once

#include <array>

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define _GLFW_WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32 1 // Request original functions explicitily.
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ICSJRenderer.h"

namespace csjruntime {

class CSJWindowSystem {
public:
    CSJWindowSystem();
    ~CSJWindowSystem();

    bool initialize();

    void* getInstacne();
    void* getSurface();

    std::array<int, 2> getWindowSize();

    void pollEvents();

    bool shouldExit();

protected:
    static void framebufferResiceCallback(GLFWwindow *window, int width, int height);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    bool initForVulkan();
    void cleanupForVulkan();

#ifdef _WIN32
    void* createSurfaceForDirectX11();
    void* createSurfaceForDirectX12();
#endif

#ifdef __APPLE__
    void* createSurfaceForMetal();
#endif

private:
    GLFWwindow *m_pWindow;

    void *m_pInstance = nullptr;
    void *m_pSurface = nullptr;

    csjrhi::CSJGraphicBackend m_graphicBackend;

    bool m_enable_validation_Layers = true;
    bool m_enable_debug_utils_label = true;
};

} // namespace csjruntime