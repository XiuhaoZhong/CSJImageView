#include "CSJRuntimeGlobalContext.h"

#include <iostream>

#include "CSJWindowSystem.h"
#include "CSJRenderSystem.h"

namespace csjruntime {

void CSJRuntimeGlobalContext::startSystem() {
    m_pWindowSystem = std::make_shared<CSJWindowSystem>();
    if (!m_pWindowSystem->initialize()) {
        std::cout << "Window system init failed." << std::endl;
    }

    m_pRenderSystem = std::make_shared<CSJRenderSystem>();
    if (!m_pRenderSystem->initialize(m_pWindowSystem.get())) {
        std::cout << "Render system initialize failed." << std::endl;
    }
}

void CSJRuntimeGlobalContext::shutdownSystem() {
    m_pRenderSystem.reset();
    m_pWindowSystem.reset();
}

} // namespace csjruntime