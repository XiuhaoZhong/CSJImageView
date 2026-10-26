#ifndef __CSJEditor_H__
#define __CSJEditor_H__

#include <vector>
#include <optional>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "CSJUILayer.h"

namespace csjruntime {
class CSJEngine;
}

class CSJEditor {
public:
    CSJEditor() = default;
    ~CSJEditor() = default;

    bool initialize(csjruntime::CSJEngine *engine);

    void run();

    void shutdown();

    // void resizeFramebuffer(int width, int height);

    // void changeEffectType();

    // void changeExposure(float delta);

    // void changeIntensity(float delta);

    // static void framebufferResiceCallback(GLFWwindow *window, int width, int height);

    // void fillContext(csjuilayer::CSJUILayerContext *context) override;
    // void* getCurrentCommandBuffer() override;

    // void render(void* commandBuffer) override;
    // void uiRendererShutdown() override;

protected:
    // bool initRenderer();
    // bool initUILayer();

    // void initWindow();

    // void mainLoop();

    // void cleanup();

    // static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    // void setEffectParam();
private:
    csjruntime::CSJEngine *m_pEngine{nullptr};
    // GLFWwindow       *m_pWindow;

    // csjrhi::ICSJRenderer   *m_pRenderer = nullptr;
    // CSJRendererLoader m_rendererLoader;

    // csjuilayer::CSJUILayer *m_pUILayer = nullptr;

    // bool m_enable_validation_Layers{ true };
    // bool m_enable_debug_utils_label{ true };

    // int m_iEffectType = 0;
    // float m_exposure = 1.0;
    // float m_intensity = 1.0;

};


#endif // __CSJEditor_H__