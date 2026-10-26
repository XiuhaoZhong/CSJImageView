#pragma once 

#include "CSJRendererLoader.h"

namespace csjruntime {

class CSJWindowSystem;

class CSJRenderSystem {
public:
    CSJRenderSystem() = default;
    ~CSJRenderSystem();

    bool initialize(CSJWindowSystem *window_system);
    void finalize();

    void tick(float timeStamp);

private:
    CSJRendererLoader m_renderLoader;
    csjrhi::ICSJRenderer* m_pRenderer;
};
} // 