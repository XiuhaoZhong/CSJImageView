#include "CSJApplication.h"

#include <fstream>
#include <set>
#include <vector>
#include <array>
#include <limits>
#include <cstring>
#include <algorithm>
#include <chrono>

// #include "ICSJRenderer.h"

#include "Utils/CSJPathTool.h"

using namespace csjrhi;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

void CSJApplication::run() {
    initWindow();
    initRenderer();
    mainLoop();
    cleanup();
}

void CSJApplication::resizeFramebuffer(int width, int height) {
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_pWindow, &width, &height);
        glfwWaitEvents();
    }
}

void CSJApplication::framebufferResiceCallback(GLFWwindow *window, int width, int height) {
    CSJApplication *app = static_cast<CSJApplication *>(glfwGetWindowUserPointer(window));
    app->resizeFramebuffer(width, height);
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

    bool res = m_pRenderer->Init(m_pWindow, WIDTH, HEIGHT);
    if (!res) {
        std::cerr << "[HostApp] Failed to initialize renderer!" << std::endl;
    } else {
        std::cout << "[HostApp] Succeed to initialize renderer!" << std::endl;
    }

    return res; 
}

void CSJApplication::initWindow() {
    std::cout << " enter init window function " << std::endl;
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResiceCallback);

#ifndef NDEBUG
    m_enable_validation_Layers = true;
    m_enable_debug_utils_label = true;
#else
    m_enable_validation_Layers = false;
    m_enable_debug_utils_label = false;
#endif
}

void CSJApplication::mainLoop() {
    while (!glfwWindowShouldClose(m_pWindow)) {
        glfwPollEvents();

        if (m_pRenderer) {
            m_pRenderer->Render();

            m_pRenderer->WaitIdle();
        }

        // TODO: call the draw function of renderer.
    }

    if (m_pRenderer) {
        m_pRenderer->Shutdown();
    }

    // TODO: call the waitIdle function of renderer.
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
