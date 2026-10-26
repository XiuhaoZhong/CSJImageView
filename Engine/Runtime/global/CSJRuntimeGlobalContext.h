#pragma once

#include <memory>

namespace csjruntime {

class CSJWindowSystem;
class CSJRenderSystem;

class CSJRuntimeGlobalContext {
public:
    CSJRuntimeGlobalContext() = default;
    ~CSJRuntimeGlobalContext() = default;

    void startSystem();
    void shutdownSystem();

    std::shared_ptr<CSJWindowSystem> getWindowSystem() {
        return m_pWindowSystem;
    }

    std::shared_ptr<CSJRenderSystem> getRenderSystem() {
        return m_pRenderSystem;
    }

protected:


private:
    std::shared_ptr<CSJWindowSystem> m_pWindowSystem{nullptr};
    std::shared_ptr<CSJRenderSystem> m_pRenderSystem{nullptr};
};

} // namespace csjruntime