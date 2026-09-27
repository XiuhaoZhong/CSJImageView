#include "CSJApplication.h"


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

#include <GLFW/glfw3native.h>

#include "CSJPathTool.h"
#include "CSJLogger.h"

using namespace csjrhi;
using namespace csjuilayer;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

CSJApplication *g_app = nullptr;

void CSJApplication::run() {
    g_app = this;

    initWindow();
    initRenderer();
    initUILayer();
    mainLoop();
    cleanup();
}

void CSJApplication::resizeFramebuffer(int width, int height) {
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_pWindow, &width, &height);
        glfwWaitEvents();
    }
}

void CSJApplication::changeEffectType() {
    m_iEffectType++;

    setEffectParam();

    m_iEffectType = m_iEffectType % static_cast<int>(CSJPostProcessEffect::Effect_Max);
}

void CSJApplication::changeExposure(float delta) {
    m_exposure += delta;

    setEffectParam();
}

void CSJApplication::changeIntensity(float delta) {
    m_intensity += delta;

    setEffectParam();
}

void CSJApplication::framebufferResiceCallback(GLFWwindow *window, int width, int height) {
    CSJApplication *app = static_cast<CSJApplication *>(glfwGetWindowUserPointer(window));
    app->resizeFramebuffer(width, height);
}

void CSJApplication::fillContext(CSJUILayerContext *context) {
    context->backendType = CSJBackendType::Vulkan;
    context->instance = m_pRenderer->GetRendererInstance();
    context->device = m_pRenderer->GetDevice();
    context->physicalDevice = m_pRenderer->GetPhysicalDevice();
    context->queue = m_pRenderer->GetQueue();
    context->queueFamilyIndex = m_pRenderer->GetQueueFamilyIndex();
    context->renderPass = m_pRenderer->GetRenderPass();
    context->descriptorPool = m_pRenderer->GetDescriptorPool();

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_pWindow, &width, &height);
    context->width = width;
    context->height = height;
}

void *CSJApplication::getCurrentCommandBuffer() {
    if (!m_pRenderer) {
        return nullptr;
    }

    return m_pRenderer->GetCurrentCommandBuffer();
}

void CSJApplication::render(void *commandBuffer) {
    if (m_pUILayer) {

        m_pUILayer->beginFrame();

        m_pUILayer->drawUI();

        m_pUILayer->render(commandBuffer);
    }
}

void CSJApplication::uiRendererShutdown() {
    if (m_pUILayer) {
        m_pUILayer->shutdown();
    }
}

bool CSJApplication::initRenderer() {
    std::string backendName = "CSJVulkanRenderer";
    //CSJRendererLoader loader;

    // Loading renderer library.
    if (!m_rendererLoader.Load(backendName)) {
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
        return false;
    }

    // Create renderer.
    m_pRenderer = m_rendererLoader.CreateRenderer();
    if (!m_pRenderer) {
        std::cerr << "[HostApp] Failed to create renderer!" << std::endl;
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
        return false;
    }

    void *windowHandle = nullptr;
#ifdef _WIN32
    windowHandle = (void *)glfwGetWin32Window(m_pWindow);
#endif
    bool res = m_pRenderer->Init(windowHandle, WIDTH, HEIGHT);
    if (!res) {
        std::cerr << "[HostApp] Failed to initialize renderer!" << std::endl;
    } else {
        std::cout << "[HostApp] Succeed to initialize renderer!" << std::endl;
    }

    m_pRenderer->setUIRendererDelegate(this);

    return res; 
}

bool CSJApplication::initUILayer() {
    CSJUILayerContext context;

    m_pUILayer = new CSJUILayer();
    bool res = m_pUILayer->initialize(this, m_pWindow);
    if (!res) {
        std::cout << "CSJUILayer initialize failed!" << std::endl;
    }

    return res;
}

void CSJApplication::initWindow() {
    std::cout << " enter init window function " << std::endl;
    glfwInit();

    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetKeyCallback(m_pWindow, keyCallback);
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResiceCallback);

    glfwSetWindowUserPointer(m_pWindow, this);

#ifndef NDEBUG
    m_enable_validation_Layers = true;
    m_enable_debug_utils_label = true;
#else
    m_enable_validation_Layers = false;
    m_enable_debug_utils_label = false;
#endif
}

void CSJApplication::mainLoop() {

    double lastFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_pWindow)) {
        glfwPollEvents();

        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime; // in second.
        double deltaTimeMs = deltaTime * 1000.0;             //  transfer to millisecond.

        lastFrameTime = currentFrameTime;

        if (m_pRenderer) {
            m_pRenderer->Render((float)deltaTimeMs);
        }
    }

    if (m_pRenderer) {
        m_pRenderer->Shutdown();
    }
}

void CSJApplication::cleanup() {
    glfwDestroyWindow(m_pWindow);

    if (m_pRenderer) {
        m_rendererLoader.DestroyRenderer(m_pRenderer);
        m_pRenderer = nullptr;
    }

    glfwTerminate();
}

void CSJApplication::createSwapChain() {
    
}

void CSJApplication::recreateSwapChain() {
    int width, height;
    glfwGetFramebufferSize(m_pWindow, &width, &height);
    while (width != 0 || height != 0) {
        glfwGetFramebufferSize(m_pWindow, &width, &height);
        glfwWaitEvents();
    }

    // TODO: call the waitIdel function and recreate functions of renderer.
}

std::vector<char> CSJApplication::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void CSJApplication::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return ;
    }

    if (!g_app) {
        return ;
    }

    if (key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL)) {
        g_app->changeEffectType();
    }

    if (key == GLFW_KEY_UP && (mods & GLFW_MOD_CONTROL)) {
        g_app->changeExposure(0.1f);
    }

    if (key == GLFW_KEY_DOWN && (mods & GLFW_MOD_CONTROL)) {
        g_app->changeExposure(-0.1f);
    }

    if (key == GLFW_KEY_RIGHT && (mods & GLFW_MOD_CONTROL)) {
        g_app->changeIntensity(0.1f);
    }

    if (key == GLFW_KEY_LEFT && (mods & GLFW_MOD_CONTROL)) {
        g_app->changeIntensity(-0.1f);
    }
}

void CSJApplication::setEffectParam() {
    if (!m_pRenderer) {
        return ;
    }

    CSJEffectParams param{};

    param.effectType = m_iEffectType;
    param.exposure = m_exposure;
    param.intensity = m_intensity;

    m_pRenderer->setPostProcessEffect(param);
}
