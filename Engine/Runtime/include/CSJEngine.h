#pragma once

#include "CSJRuntime_Export.h"

#include <chrono>
#include <memory>

namespace csjruntime {

class CSJRuntimeGlobalContext;

class CSJRUNTIME_API CSJEngine {
public:
    CSJEngine() = default;
    ~CSJEngine() = default;

    bool initlize();

    float calculateDeltaTime();

    bool tickOneFrame(float timeStamp);

    void shutdown();

private:
    std::shared_ptr<CSJRuntimeGlobalContext> m_pContext;

    int m_fps = 0;
    int m_frameCount = 0;
    float m_averageDuration = 0.0f;
    std::chrono::steady_clock::time_point m_lastTickTime{std::chrono::steady_clock::now()};
};

} // namespace csjruntime 