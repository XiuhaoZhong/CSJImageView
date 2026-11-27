#pragma once

#include "CSJRenderSystem.h"
#include "CSJWindowSystem.h"

struct CSJEditorUIInfo {
    csjruntime::CSJWindowSystem *window_system;
    csjruntime::CSJRenderSystem *render_system;
};