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

using namespace csjuilayer;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

void *createWindowSurface();

bool CSJEditor::initialize(csjruntime::CSJEngine *engine) {
    m_pEngine = engine;

    engine->initlize();

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
    
    // initWindow();
    // initRenderer();
    // initUILayer();
    // mainLoop();
    // cleanup();
}

void CSJEditor::shutdown() {
    std::cout << "Editor shutdown!" << std::endl;
}

// void CSJEditor::resizeFramebuffer(int width, int height) {
//     while (width == 0 || height == 0) {
//         glfwGetFramebufferSize(m_pWindow, &width, &height);
//         glfwWaitEvents();
//     }
// }

// void CSJEditor::changeEffectType() {
//     m_iEffectType++;

//     setEffectParam();

//     m_iEffectType = m_iEffectType % static_cast<int>(CSJPostProcessEffect::Effect_Max);
// }

// void CSJEditor::changeExposure(float delta) {
//     m_exposure += delta;

//     setEffectParam();
// }

// void CSJEditor::changeIntensity(float delta) {
//     m_intensity += delta;

//     setEffectParam();
// }

// void CSJEditor::framebufferResiceCallback(GLFWwindow *window, int width, int height) {
//     CSJEditor *app = static_cast<CSJEditor *>(glfwGetWindowUserPointer(window));
//     app->resizeFramebuffer(width, height);
// }

// void CSJEditor::fillContext(CSJUILayerContext *context) {
//     context->backendType = CSJBackendType::Vulkan;
//     context->instance = m_pRenderer->GetRendererInstance();
//     context->device = m_pRenderer->GetDevice();
//     context->physicalDevice = m_pRenderer->GetPhysicalDevice();
//     context->queue = m_pRenderer->GetQueue();
//     context->queueFamilyIndex = m_pRenderer->GetQueueFamilyIndex();
//     context->renderPass = m_pRenderer->GetRenderPass();
//     context->descriptorPool = m_pRenderer->GetDescriptorPool();

//     int width = 0, height = 0;
//     glfwGetFramebufferSize(m_pWindow, &width, &height);
//     context->width = width;
//     context->height = height;
// }

// void *CSJEditor::getCurrentCommandBuffer() {
//     if (!m_pRenderer) {
//         return nullptr;
//     }

//     return m_pRenderer->GetCurrentCommandBuffer();
// }

// void CSJEditor::render(void *commandBuffer) {
//     if (m_pUILayer) {

//         m_pUILayer->beginFrame();

//         m_pUILayer->drawUI();

//         m_pUILayer->render(commandBuffer);
//     }
// }

// void CSJEditor::uiRendererShutdown() {
//     if (m_pUILayer) {
//         m_pUILayer->shutdown();
//     }
// }

// bool CSJEditor::initRenderer() {
//     std::string backendName = "CSJVulkanRenderer";
//     //CSJRendererLoader loader;

//     // Loading renderer library.
//     if (!m_rendererLoader.Load(backendName)) {
//         glfwDestroyWindow(m_pWindow);
//         glfwTerminate();
//         return false;
//     }

//     // Create renderer.
//     m_pRenderer = m_rendererLoader.CreateRenderer();
//     if (!m_pRenderer) {
//         std::cerr << "[HostApp] Failed to create renderer!" << std::endl;
//         glfwDestroyWindow(m_pWindow);
//         glfwTerminate();
//         return false;
//     }

//     void *windowHandle = nullptr; 
// #ifdef _WIN32
//     windowHandle = (void *)glfwGetWin32Window(m_pWindow);
// #elif __APPLE__
//     windowHandle = (void *)glfwGetCocoaWindow(m_pWindow);
// #endif

//     uint32_t glfwExtensionCount = 0;
//     const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

//     std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
// #ifdef __APPLE__
//     extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
//     extensions.push_back("VK_KHR_get_physical_device_properties2");
// #endif

//     VkInstanceCreateInfo createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//     createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//     createInfo.ppEnabledExtensionNames = extensions.data();

// #ifdef __APPLE__
//     createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
// #endif

//     VkInstance instance = VK_NULL_HANDLE;
//     VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
//     if (result != VK_SUCCESS) {
//         throw std::runtime_error("failed to create instance!");
//     }

//     VkSurfaceKHR surface = VK_NULL_HANDLE;

//     result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, &surface);
//     if (surface == VK_NULL_HANDLE) {
//         std::cout << "create windows surface failed" << std::endl;
//     }

//     m_pRenderer->setWindowSurface(static_cast<void *>(surface));

//     bool res = m_pRenderer->Init(windowHandle, WIDTH, HEIGHT);
//     if (!res) {
//         std::cerr << "[HostApp] Failed to initialize renderer!" << std::endl;
//     } else {
//         std::cout << "[HostApp] Succeed to initialize renderer!" << std::endl;
//     }

//     m_pRenderer->setUIRendererDelegate(this);

//     return res; 
// }

// bool CSJEditor::initUILayer() {
//     CSJUILayerContext context;

//     m_pUILayer = new CSJUILayer();
//     bool res = m_pUILayer->initialize(this, m_pWindow);
//     if (!res) {
//         std::cout << "CSJUILayer initialize failed!" << std::endl;
//     }

//     return res;
// }

// void CSJEditor::initWindow() {
//     std::cout << " enter init window function " << std::endl;
//     glfwInit();

//     uint32_t count;
//     const char** extensions = glfwGetRequiredInstanceExtensions(&count);

//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

//     m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
//     glfwSetKeyCallback(m_pWindow, keyCallback);
//     glfwSetFramebufferSizeCallback(m_pWindow, framebufferResiceCallback);

//     glfwSetWindowUserPointer(m_pWindow, this);

// #ifndef NDEBUG
//     m_enable_validation_Layers = true;
//     m_enable_debug_utils_label = true;
// #else
//     m_enable_validation_Layers = false;
//     m_enable_debug_utils_label = false;
// #endif
// }

// void CSJEditor::mainLoop() {

//     double lastFrameTime = glfwGetTime();
//     while (!glfwWindowShouldClose(m_pWindow)) {
//         glfwPollEvents();

//         double currentFrameTime = glfwGetTime();
//         double deltaTime = currentFrameTime - lastFrameTime; // in second.
//         double deltaTimeMs = deltaTime * 1000.0;             //  transfer to millisecond.

//         lastFrameTime = currentFrameTime;

//         if (m_pRenderer) {
//             m_pRenderer->Render((float)deltaTimeMs);
//         }
//     }

//     if (m_pRenderer) {
//         m_pRenderer->Shutdown();
//     }
// }

// void CSJEditor::cleanup() {
//     glfwDestroyWindow(m_pWindow);

//     if (m_pRenderer) {
//         m_rendererLoader.DestroyRenderer(m_pRenderer);
//         m_pRenderer = nullptr;
//     }

//     glfwTerminate();
// }

// void CSJEditor::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
//     if (action != GLFW_PRESS) {
//         return ;
//     }

//     if (!g_app) {
//         return ;
//     }

//     if (key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL)) {
//         g_app->changeEffectType();
//     }

//     if (key == GLFW_KEY_UP && (mods & GLFW_MOD_CONTROL)) {
//         g_app->changeExposure(0.1f);
//     }

//     if (key == GLFW_KEY_DOWN && (mods & GLFW_MOD_CONTROL)) {
//         g_app->changeExposure(-0.1f);
//     }

//     if (key == GLFW_KEY_RIGHT && (mods & GLFW_MOD_CONTROL)) {
//         g_app->changeIntensity(0.1f);
//     }

//     if (key == GLFW_KEY_LEFT && (mods & GLFW_MOD_CONTROL)) {
//         g_app->changeIntensity(-0.1f);
//     }
// }

// void CSJEditor::setEffectParam() {
//     if (!m_pRenderer) {
//         return ;
//     }

//     CSJEffectParams param{};

//     param.effectType = m_iEffectType;
//     param.exposure = m_exposure;
//     param.intensity = m_intensity;

//     m_pRenderer->setPostProcessEffect(param);
// }
