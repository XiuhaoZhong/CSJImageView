#include "CSJEditor.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <array>
#include <limits>
#include <cstring>
#include <algorithm>
#include <chrono>

#include "CSJPathTool.h"
#include "CSJLogger.h"

#include "CSJEngine.h"
#include "CSJEditorUI.h"

#include "CSJRuntimeGlobalContext.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace csjruntime;

void *createWindowSurface();

bool CSJEditor::initialize(csjruntime::CSJEngine *engine) {
    m_pEngine = engine;

    m_pEditorUI = std::make_shared<CSJEditorUI>();
    bool res = m_pEditorUI->initialize();
    if (res) {
        res = true;
    } else {
        res = false;
    }

    CSJEditorUIInfo uiInit_info;
    uiInit_info.render_system = CSJRuntimeGlobalContext::getInstance()->getRenderSystem().get();
    uiInit_info.window_system = CSJRuntimeGlobalContext::getInstance()->getWindowSystem().get();
    m_pEditorUI->setRenderBackend(&uiInit_info);

    return true;
}

void CSJEditor::run() {
    if (!m_pEngine) {
        return ;
    }

    while (true) {
        float time_span = m_pEngine->calculateDeltaTime();
        if (m_pEngine->tickOneFrame(time_span)) {
            return ;
        }
    }
}

void CSJEditor::shutdown() {
    std::cout << "Editor shutdown!" << std::endl;
}
