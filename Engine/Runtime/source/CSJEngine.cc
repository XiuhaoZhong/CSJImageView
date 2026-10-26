#include "CSJEngine.h"

#include <iostream>

#include "CSJRuntimeGlobalContext.h"

#include "CSJWindowSystem.h"
#include "CSJRenderSystem.h"

using namespace std::chrono;

namespace csjruntime {

bool CSJEngine::initlize() {

    m_pContext = std::make_shared<CSJRuntimeGlobalContext>();
    m_pContext->startSystem();

    return true;
}

float CSJEngine::calculateDeltaTime() {

    float delta_time;

    steady_clock::time_point current_point = steady_clock::now();
    duration<float> time_span = duration_cast<duration<float>>(current_point - m_lastTickTime);
    delta_time = time_span.count();

    m_lastTickTime = current_point;

    return delta_time;
}

bool CSJEngine::tickOneFrame(float timeStamp) {

    // TODO: logic tick and render tick.
    float delta_time = calculateDeltaTime();

    m_pContext->getRenderSystem()->tick(delta_time);

    m_pContext->getWindowSystem()->pollEvents();

    return m_pContext->getWindowSystem()->shouldExit();
}

void CSJEngine::shutdown() {
    std::cout << "Engine shutdown" << std::endl;
    if (m_pContext) {
        m_pContext->shutdownSystem();
    }
}

} // namespace csjruntime