#pragma once

#include <GLFW/glfw3.h>

#include "ICSJUIPass.h"

using csjrhi::CSJUIPassContext;

namespace csjruntime {

class ICSJWindowUI;

class CSJUIPassImpl : public csjrhi::ICSJUIPass {
public:
    bool initialize(ICSJWindowUI *windowUI, CSJUIPassContext *context, GLFWwindow* window);
    void beginFrame();
    void endFrame();
    void drawUI();

    void render(void *cmd) override;
    void shutdown() override;

protected:
    bool initForVulkan(CSJUIPassContext *context);

private:
    bool m_bInit = false;
    ICSJWindowUI         *m_pWindowUI = nullptr;
    GLFWwindow           *m_pWindow = nullptr;
};

using CSJUIPassImplPtr = std::shared_ptr<CSJUIPassImpl>;

} // namespace csjuilayer