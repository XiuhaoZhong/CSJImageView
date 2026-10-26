#pragma once

#include <GLFW/glfw3.h>

#include "CSJUILayerContext.h"

#include "ImGui/imgui.h"

namespace csjuilayer {

class ICSJUILayerContextDelegate {
public: 
    ICSJUILayerContextDelegate() = default;
    ~ICSJUILayerContextDelegate() = default;

    virtual void fillContext(CSJUILayerContext *context) = 0;
    virtual void* getCurrentCommandBuffer() = 0;
};

class CSJUILayer {
public:
    bool initialize(ICSJUILayerContextDelegate *delegate, GLFWwindow* window);
    void beginFrame();
    void endFrame();
    void drawUI();
    void render(void *cmd);
    void shutdown();

protected:
    void initForVulkan();

private:
    bool m_bInit = false; 
    ICSJUILayerContextDelegate *m_pDelegate = nullptr;
    CSJUILayerContext m_context;
    GLFWwindow *m_pWindow = nullptr;
};

} // namespace csjuilayer