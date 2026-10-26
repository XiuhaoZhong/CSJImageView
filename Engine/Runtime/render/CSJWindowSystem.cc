#include "CSJWindowSystem.h"

#include <iostream>

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace csjrhi;

namespace csjruntime {

CSJWindowSystem::CSJWindowSystem() {

}

CSJWindowSystem::~CSJWindowSystem() {
    if (m_graphicBackend == CSJGraphicBackend::CSJGraphicBackend_Vulkan) {
        cleanupForVulkan();
    }

    glfwDestroyWindow(m_pWindow);
    glfwTerminate();

    std::cout << "Window system deconstructed." << std::endl;
}

bool CSJWindowSystem::initialize() {
    std::cout << " enter initialize window function " << std::endl;
    glfwInit();

    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "CSJRuntime", nullptr, nullptr);
    glfwSetWindowUserPointer(m_pWindow, this);
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

    bool result = false;
    if (m_graphicBackend == CSJGraphicBackend::CSJGraphicBackend_Vulkan) {
        result = initForVulkan();
    }

    return result;
}

void *CSJWindowSystem::getInstacne() {
    return m_pInstance;
}

void *CSJWindowSystem::getSurface() {
    return m_pSurface;
}

std::array<int, 2> CSJWindowSystem::getWindowSize() {
    int width = 0, height = 0;

    if (m_pWindow) {
        glfwGetWindowSize(m_pWindow, &width, &height);
    }

    return {width, height};
}

void CSJWindowSystem::pollEvents() {
    glfwPollEvents();
}

bool CSJWindowSystem::shouldExit() {
    return glfwWindowShouldClose(m_pWindow);
}

void CSJWindowSystem::framebufferResiceCallback(GLFWwindow *window, int width, int height) {
    CSJWindowSystem *window_system = static_cast<CSJWindowSystem *>(glfwGetWindowUserPointer(window));
    if (!window_system) {
        return ;
    }

    // std::cout << "Window change size, new size: " << width << "x" << height << std::endl;
}

void CSJWindowSystem::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return ;
    }

    std::cout << "Pressed a key" << std::endl;

    // if (!g_app) {
    //     return ;
    // }

    // if (key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL)) {
    //     g_app->changeEffectType();
    // }

    // if (key == GLFW_KEY_UP && (mods & GLFW_MOD_CONTROL)) {
    //     g_app->changeExposure(0.1f);
    // }

    // if (key == GLFW_KEY_DOWN && (mods & GLFW_MOD_CONTROL)) {
    //     g_app->changeExposure(-0.1f);
    // }

    // if (key == GLFW_KEY_RIGHT && (mods & GLFW_MOD_CONTROL)) {
    //     g_app->changeIntensity(0.1f);
    // }

    // if (key == GLFW_KEY_LEFT && (mods & GLFW_MOD_CONTROL)) {
    //     g_app->changeIntensity(-0.1f);
    // }
}

bool CSJWindowSystem::initForVulkan() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, &surface);
    if (surface == VK_NULL_HANDLE) {
        std::cout << "create windows surface failed" << std::endl;
    }

    m_pInstance = static_cast<void *>(instance);
    m_pSurface = static_cast<void *>(surface);

    return true;
}

void CSJWindowSystem::cleanupForVulkan() {
    VkInstance inst = static_cast<VkInstance>(m_pInstance);
    VkSurfaceKHR surface = static_cast<VkSurfaceKHR>(m_pSurface);

    vkDestroySurfaceKHR(inst, surface, nullptr);
    vkDestroyInstance(inst, nullptr);

    m_pInstance = nullptr;
    m_pSurface = nullptr;
}

} // namespace csjruntime