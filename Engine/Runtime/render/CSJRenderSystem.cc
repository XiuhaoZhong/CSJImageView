#include "CSJRenderSystem.h"

#include <iostream>

#include "CSJWindowSystem.h"

namespace csjruntime {
CSJRenderSystem::~CSJRenderSystem() {
    finalize();
}

bool CSJRenderSystem::initialize(CSJWindowSystem *window_system) {
    if (!m_renderLoader.Load("CSJVulkanRenderer")) {
        std::cout << "load renderer libray failed." << std::endl;
        return false;
    }

    m_pRenderer = m_renderLoader.CreateRenderer();
    if (!m_pRenderer) {
        std::cout << "create renderer failed." << std::endl;
        return false;
    }

    auto windowSize = window_system->getWindowSize();
    if (!m_pRenderer->initialize(window_system->getInstacne(), window_system->getSurface(), windowSize)) {
        std::cout << "Initialize renderer failed!" << std::endl;
        return false;
    }

    return true;
}

void CSJRenderSystem::finalize() {
    if (m_pRenderer) {
        m_pRenderer->Shutdown();
        m_renderLoader.DestroyRenderer(m_pRenderer);
        m_pRenderer = nullptr;
    }
}

void CSJRenderSystem::tick(float timeStamp) {
    if (!m_pRenderer) {
        std::cout << "Renderer is empy, skip tick!" << std::endl;
        return ;
    }

    m_pRenderer->Render(timeStamp);
}

} // namespace csjruntime