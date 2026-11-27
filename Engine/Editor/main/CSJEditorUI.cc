#include "CSJEditorUI.h"

#include <imgui.h>

CSJEditorUI::CSJEditorUI() {

}

CSJEditorUI::~CSJEditorUI() {

}

bool CSJEditorUI::initialize() {
    return true;
}

void CSJEditorUI::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::Begin("ImGui_Demo");
    ImGui::Text("Hello ImGui!") ;
    ImGui::End();

    ImGui::Begin("ImGui_Demo_1");
    ImGui::Text("Hello ImGui 1!") ;
    ImGui::End();
}

void CSJEditorUI::setRenderBackend(CSJEditorUIInfo *uiInfo) {
    if (uiInfo->render_system) {
        uiInfo->render_system->setWindowUIBackend(this);
    }
}
