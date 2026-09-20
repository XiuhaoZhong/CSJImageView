#include "CSJUILayer.h"

#include <iostream>

#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_vulkan.h"

namespace csjuilayer {

bool CSJUILayer::initialize(ICSJUILayerContextDelegate *delegate, GLFWwindow *window) {

    ImDrawCmdHeader cmdHeader{};

    m_pDelegate = delegate;
    m_pWindow = window;

    if (m_pDelegate) {
        m_pDelegate->fillContext(&m_context);
    }

    initForVulkan();

    m_bInit = true;

    return true;
}

void CSJUILayer::beginFrame() {
    if (!m_bInit) {
        return ;
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void CSJUILayer::endFrame() {
    if (!m_bInit) {
        return ;
    }

    ImGui::Render();
}

void CSJUILayer::drawUI() {
    if (!m_bInit) {
        return ;
    }

    ImGui::Begin("ImGui");
    ImGui::Text("Hello ImGui!") ;
    ImGui::End();
}

void CSJUILayer::render(void *cmd) {
    if (!m_bInit) {
        return ;
    }

    VkCommandBuffer commandBuffer = static_cast<VkCommandBuffer>(m_pDelegate->getCurrentCommandBuffer());

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void CSJUILayer::shutDown() {
    if (!m_bInit) {
        return ;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CSJUILayer::initForVulkan() {
    // 1. 创建 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // 2. 初始化 GLFW 平台后端
    ImGui_ImplGlfw_InitForVulkan(m_pWindow, true);

    // 3. 填充 Vulkan 初始化信息（从 context 中转换 void*）
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = static_cast<VkInstance>(m_context.instance);
    init_info.PhysicalDevice = static_cast<VkPhysicalDevice>(m_context.physicalDevice);
    init_info.Device = static_cast<VkDevice>(m_context.device);
    init_info.QueueFamily = m_context.queueFamilyIndex;
    init_info.Queue = static_cast<VkQueue>(m_context.queue);
    init_info.DescriptorPool = static_cast<VkDescriptorPool>(m_context.descriptorPool);
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.PipelineInfoMain.RenderPass = static_cast<VkRenderPass>(m_context.renderPass);

    // 4. 初始化 Vulkan 后端
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        std::cerr << "ImGui initialize failed!" << std::endl;
    }
}

} // namespace csjuilayer