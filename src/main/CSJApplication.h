#ifndef __CSJAPPLICATION_H__
#define __CSJAPPLICATION_H__

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define _GLFW_WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32 1 // Request original functions explicitily.
#endif

#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "CSJRendererLoader.h"

class CSJApplication {
public:
    CSJApplication() = default;
    ~CSJApplication() = default;
    void run();

    void setFramebufferResize(bool framebufferResize) {
        m_bFrameBufferResize = framebufferResize;
    }

    void resizeFramebuffer(int width, int height);

    static void framebufferResiceCallback(GLFWwindow *window, int width, int height);

protected:
    bool initRenderer();

    void initWindow();

    void mainLoop();

    void cleanup();

    void createSwapChain();
    void recreateSwapChain();

    std::vector<char> readFile(const std::string& filename);
private:
    GLFWwindow       *m_pWindow;
    bool              m_bFrameBufferResize = false;
    csjrhi::ICSJRenderer     *m_pRenderer = nullptr;
    CSJRendererLoader m_rendererLoader;

    bool m_enable_validation_Layers{ true };
    bool m_enable_debug_utils_label{ true };

};


#endif // __CSJAPPLICATION_H__