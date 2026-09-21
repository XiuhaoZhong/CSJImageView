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
#include "CSJUILayer.h"
#include "ICSJRenderer.h"

class CSJApplication : public csjuilayer::ICSJUILayerContextDelegate,
                       public csjrhi::ICSJUIRendererDelegate {
public:
    CSJApplication() = default;
    ~CSJApplication() = default;
    void run();

    void setFramebufferResize(bool framebufferResize) {
        m_bFrameBufferResize = framebufferResize;
    }

    void resizeFramebuffer(int width, int height);

    void changeEffectType();

    void changeExposure(float delta);

    void changeIntensity(float delta);

    static void framebufferResiceCallback(GLFWwindow *window, int width, int height);

    void fillContext(csjuilayer::CSJUILayerContext *context) override;
    void* getCurrentCommandBuffer() override;

    void render(void* commandBuffer) override;
    void uiRendererShutdown() override;

protected:
    bool initRenderer();
    bool initUILayer();

    void initWindow();

    void mainLoop();

    void cleanup();

    void createSwapChain();
    void recreateSwapChain();

    std::vector<char> readFile(const std::string& filename);

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void setEffectParam();
private:
    GLFWwindow       *m_pWindow;
    bool              m_bFrameBufferResize = false;

    csjrhi::ICSJRenderer   *m_pRenderer = nullptr;
    CSJRendererLoader m_rendererLoader;

    csjuilayer::CSJUILayer *m_pUILayer = nullptr;

    bool m_enable_validation_Layers{ true };
    bool m_enable_debug_utils_label{ true };

    int m_iEffectType = 0;
    float m_exposure = 1.0;
    float m_intensity = 1.0;

};


#endif // __CSJAPPLICATION_H__