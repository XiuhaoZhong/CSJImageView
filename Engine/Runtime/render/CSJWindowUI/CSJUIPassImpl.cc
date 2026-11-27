#include "CSJUIPassImpl.h"

#include <iostream>

#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_vulkan.h"

#include "CSJLogger.h"
#include "ICSJWindowUI.h"

namespace csjruntime {

bool CSJUIPassImpl::initialize(ICSJWindowUI *windowUI, CSJUIPassContext *context, GLFWwindow *window) {
    m_pWindowUI = windowUI;
    m_pWindow = window;

    bool res = initForVulkan(context);
    if (!res) {
        return res;
    }

    m_bInit = true;

    return true;
}

void CSJUIPassImpl::beginFrame() {
    if (!m_bInit) {
        return ;
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void CSJUIPassImpl::endFrame() {
    if (!m_bInit) {
        return ;
    }

    ImGui::Render();
}

void CSJUIPassImpl::drawUI() {
    if (!m_bInit) {
        return ;
    }

    beginFrame();
    if (m_pWindowUI) {
        m_pWindowUI->renderUI();
    }
}

void CSJUIPassImpl::render(void *cmd) {
    if (!m_bInit) {
        return ;
    }

    VkCommandBuffer commandBuffer = static_cast<VkCommandBuffer>(cmd);

    drawUI();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void CSJUIPassImpl::shutdown() {
    if (!m_bInit) {
        return ;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool CSJUIPassImpl::initForVulkan(CSJUIPassContext *context) {
    if (!context) {
        return false;
    }

    // create ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // initialize GLFW backend.
    /**
     * There is a crash due to the m_pWindow is null, I don't handle it when the uipass initialize. 2026-11-27.
     */
    ImGui_ImplGlfw_InitForVulkan(m_pWindow, true);

    // Fill context from renderer.
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                    = static_cast<VkInstance>(context->instance);
    init_info.PhysicalDevice              = static_cast<VkPhysicalDevice>(context->physicalDevice);
    init_info.Device                      = static_cast<VkDevice>(context->device);
    init_info.QueueFamily                 = context->queueFamilyIndex;
    init_info.Queue                       = static_cast<VkQueue>(context->queue);
    init_info.DescriptorPool              = static_cast<VkDescriptorPool>(context->descriptorPool);
    init_info.MinImageCount               = 2;
    init_info.ImageCount                  = 2;
    init_info.PipelineInfoMain.RenderPass = static_cast<VkRenderPass>(context->renderPass);

    // Initialize Vulkan backend.
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        std::cerr << "ImGui initialize failed!" << std::endl;
        return false;
    }

    return true;
}

} // namespace CSJUIPassImpl