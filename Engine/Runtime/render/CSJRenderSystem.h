#pragma once 

#include "CSJRendererLoader.h"

namespace csjrhi {
class ICSJUIPass;
}

namespace csjruntime {

class CSJWindowSystem;
class ICSJWindowUI;
class CSJUIPassImpl;

class CSJRenderSystem {
public:
    CSJRenderSystem() = default;
    ~CSJRenderSystem();

    bool initialize(CSJWindowSystem *window_system);
    void finalize();

    void setWindowUIBackend(ICSJWindowUI *windowUI);

    void tick(float timeStamp);

private:
    CSJRendererLoader m_renderLoader;
    csjrhi::ICSJRenderer* m_pRenderer;

    std::shared_ptr<CSJUIPassImpl> m_pUIPassImpl = nullptr;
};
} // 