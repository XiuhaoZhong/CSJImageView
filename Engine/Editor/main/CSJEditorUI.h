#pragma once

#include "CSJWindowUI/ICSJWindowUI.h"

#include "CSJEditorGlobalContext.h"

class CSJEditorUI : public csjruntime::ICSJWindowUI {
public:
    CSJEditorUI();
    ~CSJEditorUI();

    bool initialize() override;
    void renderUI() override;

    void setRenderBackend(CSJEditorUIInfo *uiInfo);
};