#include "CSJVulkanRenderer.h"

#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <thread>
#include <chrono>
#include <array>

#ifdef _WIN32
#include <windows.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "stb_image.h"

namespace csjrhi {

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

const std::vector<Vertex> img_vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0,1,2,2,3,0
}; 

// debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                    VkDebugUtilsMessageTypeFlagsEXT,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

CSJVulkanRenderer::~CSJVulkanRenderer() {

}

bool CSJVulkanRenderer::Init(void *windowHandle, int width, int height) {
    if (!windowHandle || width == 0 || height == 0) {
        return false;
    }

    m_pWindow = windowHandle;

    initVulkan();

    std::cout << "CSJVulkan Renderer initialize success." << std::endl;
    return true;
}

void CSJVulkanRenderer::Shutdown() {
    std::cout << "CSJVulkan Renderer shutdown." << std::endl;

    if (m_pHelper) {
        m_pHelper = nullptr;
    }

    cleanup();
}

void CSJVulkanRenderer::Resize(int width, int height) {
    recreateSwapChain();
}

void CSJVulkanRenderer::Render() {
    auto windowSize = getCurrentWindowSize();
    if (m_windowWidth != windowSize[0] || m_windowHeight != windowSize[1]) {
        m_windowWidth = windowSize[0];
        m_windowHeight = windowSize[1];
        m_bNeedRecreateSwapChain = true;
    }

    if (m_bNeedRecreateSwapChain) {
        std::cout << "Recreate swapchain" << std::endl;
        recreateSwapChain();
        m_bNeedRecreateSwapChain = false;
        return ;
    }

    drawFrame();
}

void CSJVulkanRenderer::WaitIdle() {

}

uint32_t CSJVulkanRenderer::CreateTexture(int width, int height, int format, const void *data) {
    return 0;
}

void CSJVulkanRenderer::DestroyTexture(uint32_t textureId) {

}

void CSJVulkanRenderer::UpdateTexture(uint32_t textureId, const void *data) {
}

std::string CSJVulkanRenderer::GetBackendName() const {
    return std::string();
}

float CSJVulkanRenderer::GetLastFrameTime() const {
    return 0.0f;
}

const CSJRendererCapabilities &CSJVulkanRenderer::GetCapabilities() const {
    // TODO: insert return statement here
    return m_renderCaps;
}

void CSJVulkanRenderer::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createVulkanHelper();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createTextureImage();
    createUniformBuffers();
    createVertexBuffer();
    createIndexBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffer();
    createSyncObjects();

    initYUVComputeComponents();
}

std::array<int, 2> CSJVulkanRenderer::getCurrentWindowSize() {
    if (!m_pWindow) {
        return {0, 0};
    }

#ifdef _WIN32
    RECT rect;
    GetClientRect((HWND)m_pWindow, &rect);
    return {rect.right - rect.left, rect.bottom - rect.top};
#endif

    return {0, 0};
}

void CSJVulkanRenderer::cleanup() {

    if (m_device == VK_NULL_HANDLE) {
        return ;
    }

    vkDeviceWaitIdle(m_device);

    if (m_enable_validation_Layers) {
        
    }

    cleanupSwapChain();

    vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);

    for (size_t i = 0 ; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_device, m_uniform_buffers[i], nullptr);
        vkFreeMemory(m_device, m_uniform_buffer_memories[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

    /**
     * The TextureData object holds a vkDevice instance, so the object must
     * be released before the vkDevice is released.
     */
    if (m_pTexData) {
        m_pTexData.reset();
    }

    vkDestroyDescriptorSetLayout(m_device, m_descriptorset_layout, nullptr);

    vkDestroyBuffer(m_device, m_index_buffer, nullptr);
    vkFreeMemory(m_device, m_index_buffer_memory, nullptr);

    vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);

    unInitYUVComputeComponents();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device, m_render_finish_semas[i], nullptr);
        vkDestroySemaphore(m_device, m_image_available_semas[i], nullptr);
        vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    vkDestroyDevice(m_device, nullptr);

    vkDestroySurfaceKHR(m_VkInstance, m_surface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
}

void CSJVulkanRenderer::cleanupSwapChain() {
    for (auto framebuffer : m_swapchain_frame_buffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    for (auto imageView : m_swapchain_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void CSJVulkanRenderer::createInstance() {
    if (m_enable_validation_Layers && !checkValidationLayerSupport()) {
        return;
    }

    std::cout << " enter create instance function " << std::endl;

    // The followed info is mainly for debug tools such as RenderDoc and so on.
    // Remvoing the appInfo doesn't influence the rendering functionality.
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "CSJVulkanRenderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "CSJVulkanRenderer";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
#ifdef __APPLE__ 
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif 

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_enable_validation_Layers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
        createInfo.ppEnabledLayerNames = m_validation_layers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VkInstance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

bool CSJVulkanRenderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : m_validation_layers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char *> CSJVulkanRenderer::getRequiredExtensions() {
    std::vector<const char *> extensions;

    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    if (m_enable_validation_Layers || m_enable_debug_utils_label) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef _WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
    extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);  // 或 XLIB
#endif

#if defined(__MACH__)
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions.push_back( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
#endif
    return extensions;
}

void CSJVulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void CSJVulkanRenderer::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

    for (const auto &device : devices) {
        if (isDeviceSuitable(device)) {
            m_physical_device = device;
            break;
        }
    }

    if (m_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool CSJVulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices CSJVulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.m_graphics_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

        if (presentSupport) {
            indices.m_present_family = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void CSJVulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(m_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.m_graphics_family.value(),
        indices.m_present_family.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_device_extensions.size());
    createInfo.ppEnabledExtensionNames = m_device_extensions.data();

    if (m_enable_validation_Layers) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(m_validation_layers.size());
        createInfo.ppEnabledLayerNames = m_validation_layers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physical_device, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, indices.m_graphics_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.m_present_family.value(), 0, &m_present_queue);
}

void CSJVulkanRenderer::createVulkanHelper() {
    m_pHelper = std::make_unique<CSJVulkanHelper>(m_device, m_physical_device, m_graphics_queue);
}

void CSJVulkanRenderer::createSurface() {
    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
#ifdef _WIN32
    HWND hwnd = (HWND)m_pWindow;
    if (!hwnd) {
        return ;
    }

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = hInstance;
    surfaceInfo.hwnd = hwnd;
#endif

    VkResult result = vkCreateWin32SurfaceKHR(m_VkInstance, &surfaceInfo, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanRenderer] Failed to create Win32 surface! Error: " << result << std::endl;
        return ;
    }

    std::cout << "[VulkanRenderer] Surface created successfully." << std::endl;
}

void CSJVulkanRenderer::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physical_device);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount > 0 && 
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_physical_device);
    uint32_t queueFamilyIndices[] = {indices.m_graphics_family.value(), indices.m_present_family.value()};

    if (indices.m_graphics_family != indices.m_present_family) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchain_images.data());

    m_swapchain_image_format = surfaceFormat.format;
    m_swapchain_extent = extent;

    m_windowWidth = extent.width;
    m_windowHeight = extent.height;
}

void CSJVulkanRenderer::recreateSwapChain() {
    vkDeviceWaitIdle(m_device);

    if (m_windowWidth == 0 || m_windowHeight == 0) {
        m_bNeedRecreateSwapChain = true;
        return ;
    }

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFrameBuffers();
}

void CSJVulkanRenderer::createImageViews() {
    m_swapchain_imageViews.resize(m_swapchain_images.size());

    for (size_t i = 0; i < m_swapchain_images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = m_swapchain_images[i];
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = m_swapchain_image_format;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchain_imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

std::vector<char> CSJVulkanRenderer::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cout << "spv file: " << filename << std::endl;
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule CSJVulkanRenderer::createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }

    return shaderModule;
}

void CSJVulkanRenderer::createGraphicsPipeline() {
    auto vertShaderCode = readFile("resources/shaders/vert.spv");
    auto fragShaderCode = readFile("resources/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable  = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                            VK_COLOR_COMPONENT_G_BIT | 
                                            VK_COLOR_COMPONENT_B_BIT | 
                                            VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &m_descriptorset_layout;
    //pipelineLayoutInfo.pushConstantRangeCount = 0;
    
    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, 
                               nullptr, &m_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = m_pipeline_layout;
    pipelineInfo.renderPass          = m_render_pass;
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, 
                                  &pipelineInfo, nullptr, 
                                  &m_graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void CSJVulkanRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format          = m_swapchain_image_format;
    colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void CSJVulkanRenderer::createFrameBuffers() {
    m_swapchain_frame_buffers.resize(m_swapchain_imageViews.size());

    for (size_t i = 0; i < m_swapchain_imageViews.size(); i++) {
        VkImageView attachments[] = {
            m_swapchain_imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchain_extent.width;
        framebufferInfo.height = m_swapchain_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, 
                                &m_swapchain_frame_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create frame buffer!");
        }
    }
}

void CSJVulkanRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physical_device);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.m_graphics_family.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool");
    }
}

void CSJVulkanRenderer::createCommandBuffer() {
    m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_command_pool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = m_command_buffers.size();

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command buffer!");
    }
}

void CSJVulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    generateYUVFrame(commandBuffer, m_time);
    TransitionYUVToSampling(commandBuffer);
    m_time += 0.016;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = m_render_pass;
    renderPassInfo.framebuffer       = m_swapchain_frame_buffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchain_extent;

    VkClearValue clearColor        = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues    = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_yuvPipeline);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_yuvPipelineLayout,
                            0,
                            1,
                            &m_yuvDescriptorSet,
                            0,
                            nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);


    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float) m_swapchain_extent.width;
    viewport.height   = (float) m_swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain_extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);    

    VkBuffer vertexBuffers[] = {
        m_vertex_buffer
    };        
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout, 0, 1, &m_descriptor_sets[m_current_frame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0,  0);
    //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to crecord command");
    }
}

void CSJVulkanRenderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_image_available_semas.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finish_semas.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_image_available_semas[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_render_finish_semas[i]) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void CSJVulkanRenderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding            = 0;
    uboLayoutBinding.descriptorCount    = 1;
    uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding            = 1;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorset_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

void CSJVulkanRenderer::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create diecriptor pool!");
    }
}

void CSJVulkanRenderer::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorset_layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts        = layouts.data();

    m_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniform_buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = static_cast<VkImageView>(m_pTexData->GetNativeHandle());
        imageInfo.sampler     = static_cast<VkSampler>(m_pTexData->GetSampler());

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet          = m_descriptor_sets[i];
        descriptorWrites[0].dstBinding      = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet          = m_descriptor_sets[i];
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(m_device, 
                               static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(),
                               0, nullptr);
    }
}

void CSJVulkanRenderer::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniform_buffer_memories.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniform_buffer_mappeds.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0 ; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_uniform_buffers[i],
                     m_uniform_buffer_memories[i]);
        
        vkMapMemory(m_device, m_uniform_buffer_memories[i], 0, bufferSize, 0, &m_uniform_buffer_mappeds[i]);
    }
}

void CSJVulkanRenderer::createVertexBuffer() {

    std::vector<Vertex> current_vertices = img_vertices;
    VkDeviceSize bufferSize = sizeof(current_vertices[0]) * current_vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, 
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, current_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(bufferSize, 
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_vertex_buffer,
                 m_vertex_buffer_memory);
    
    copyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void CSJVulkanRenderer::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(bufferSize, 
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_index_buffer, m_index_buffer_memory);

    copyBuffer(stagingBuffer, m_index_buffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void CSJVulkanRenderer::createTextureImage() {
    std::string imagePath("E:\\technology\\Personal_Projects\\CSJImageView\\resources\\originImages\\slamDumk_images\\cross_street.jpeg");

    m_pTexData = m_pHelper->CreateTextureFromFile(imagePath);
}

void CSJVulkanRenderer::createBuffer(VkDeviceSize size,
                                     VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties,
                                     VkBuffer &buffer,
                                     VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void CSJVulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_device, m_command_pool, 1, &commandBuffer);
}

void CSJVulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model    = glm::rotate(glm::mat4(1.0f), 0.0f/*time * glm::radians(90.0f)*/, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view     = glm::mat4(1.0f);//glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    float aspect = (float) m_swapchain_extent.width / (float) m_swapchain_extent.height;
    ubo.proj     = glm::mat4(1.0f);//glm::perspective(glm::radians(1.0f), aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_uniform_buffer_mappeds[m_current_frame], &ubo, sizeof(ubo));
}

void CSJVulkanRenderer::drawFrame() {
    vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device,
                                            m_swapchain, 
                                            UINT64_MAX, 
                                            m_image_available_semas[m_current_frame], 
                                            VK_NULL_HANDLE, 
                                            &imageIndex);
    if (result != VK_SUCCESS) {
        std::cout << "Require next image failed!" << std::endl;
        return ;
    }

    updateUniformBuffer(m_current_frame);

    vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
    recordCommandBuffer(m_command_buffers[m_current_frame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_image_available_semas[m_current_frame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_command_buffers[m_current_frame];

    VkSemaphore signalSemaphores[] = {m_render_finish_semas[m_current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_graphics_queue, 1, &submitInfo, m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_present_queue, &presentInfo);

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkSurfaceFormatKHR CSJVulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR CSJVulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D CSJVulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        auto windowSize = getCurrentWindowSize();

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(windowSize[0]),
            static_cast<uint32_t>(windowSize[1])
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainSupportDetails CSJVulkanRenderer::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool CSJVulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_device_extensions.begin(), m_device_extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

uint32_t CSJVulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void CSJVulkanRenderer::createyuvDescriptorPool() {
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[0].descriptorCount = 3;  // Y, U, V
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;  // compute uniform
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = 3;  // Y, U, V
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[3].descriptorCount = 1;  // graphics uniform (optional)

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 2;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_yuvDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void CSJVulkanRenderer::createYUVComputeDescriptorSetLayout()
{
    // 3 storage images + 1 uniform buffer
    std::array<VkDescriptorSetLayoutBinding, 4> bindings{};

    // Binding 0: Y storage image
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    // Binding 1: U storage image
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 2: V storage image
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 3: Uniform buffer
    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_yuvComputeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }
}

void CSJVulkanRenderer::createYUVComputePipelineLayout() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_yuvComputeDescriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_yuvComputePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }
}

void CSJVulkanRenderer::createYUVComputePipeline() {
     // 1. Load shader module
    auto shaderCode = readFile("resources/shaders/yuv_generator.spv");
    VkShaderModule shaderModule = createShaderModule(shaderCode);

    // 2. Shader stage info
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = shaderModule;
    stageInfo.pName = "main";

    // 3. Create pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = m_yuvComputePipelineLayout;

    if (vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_yuvComputePipeline) != VK_SUCCESS) {
        vkDestroyShaderModule(m_device, shaderModule, nullptr);
        throw std::runtime_error("failed to create compute pipeline!");
    }

    // 4. Cleanup shader module (no longer needed)
    vkDestroyShaderModule(m_device, shaderModule, nullptr);

    std::cout << "[VulkanRenderer] Compute pipeline created." << std::endl;
}

void CSJVulkanRenderer::createYUVStorageImages(uint32_t width, uint32_t height) {
    //  // Y plane: full resolution
    // m_yuvYImage = createYUVStorageImage(width, height, VK_FORMAT_R8_UNORM);
    // m_yuvUImage = createYUVStorageImage(width / 2, height / 2, VK_FORMAT_R8_UNORM);
    // m_yuvVImage = createYUVStorageImage(width / 2, height / 2, VK_FORMAT_R8_UNORM);
}

VkImage CSJVulkanRenderer::createYUVStorageImage(uint32_t width, uint32_t height, VkFormat format) {
    // VkImageCreateInfo imageInfo{};
    // imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    // imageInfo.imageType = VK_IMAGE_TYPE_2D;
    // imageInfo.extent.width = width;
    // imageInfo.extent.height = height;
    // imageInfo.extent.depth = 1;
    // imageInfo.mipLevels = 1;
    // imageInfo.arrayLayers = 1;
    // imageInfo.format = format;
    // imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    // imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage image;
    // if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to create storage image!");
    // }

    // Allocate and bind memory...
    // (same as texture creation, but with STORAGE_BIT usage)

    return image;
}

void CSJVulkanRenderer::createYUVComputeDescriptorSet() {
    // 1. Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_yuvDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_yuvComputeDescriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_yuvComputeDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute descriptor set!");
    }

    // 2. Update descriptor set with storage images and uniform buffer
    std::array<VkDescriptorImageInfo, 3> imageInfos{};
    imageInfos[0].imageView   = m_yuvYImageView;  // Create image views for storage images
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;  // Storage images use GENERAL layout
    imageInfos[1].imageView   = m_yuvUImageView;
    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfos[2].imageView   = m_yuvVImageView;
    imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_yuvUniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(YUVUniforms);

    std::array<VkWriteDescriptorSet, 4> writes{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = m_yuvComputeDescriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[0].pImageInfo = &imageInfos[0];

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = m_yuvComputeDescriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[1].pImageInfo = &imageInfos[1];

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = m_yuvComputeDescriptorSet;
    writes[2].dstBinding = 2;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[2].pImageInfo = &imageInfos[2];

    writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[3].dstSet = m_yuvComputeDescriptorSet;
    writes[3].dstBinding = 3;
    writes[3].descriptorCount = 1;
    writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[3].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

// void CSJVulkanRenderer::createYUVComputeDescriptorPool() {
//     // 1. Define the pool sizes (what types and how many)
//     std::array<VkDescriptorPoolSize, 4> poolSizes{};

//     // Storage images (Y, U, V)
//     poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
//     poolSizes[0].descriptorCount = 3;

//     // Uniform buffer (time, aspect ratio)
//     poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     poolSizes[1].descriptorCount = 1;

//     // Combined image samplers (for fragment shader)
//     poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//     poolSizes[2].descriptorCount = 3;

//     // Uniform buffers (for graphics pipeline)
//     poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     poolSizes[3].descriptorCount = 1;

//     // 2. Create the pool
//     VkDescriptorPoolCreateInfo poolInfo{};
//     poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//     poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//     poolInfo.maxSets = 2;  // 1 for compute, 1 for graphics (or more if needed)
//     poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//     poolInfo.pPoolSizes = poolSizes.data();

//     if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_yuvComputeDescirptorPool) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create descriptor pool!");
//     }
// }

void CSJVulkanRenderer::TransitionYUVToSampling(VkCommandBuffer commandBuffer) {
    auto barrier = [&](VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        } else {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
        }

        return barrier;
    };

    std::array<VkImageMemoryBarrier, 3> barriers{
        barrier(m_yuvYImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        barrier(m_yuvUImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        barrier(m_yuvVImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    };

    // Source stage: compute shader (the compute shader wrote to the images)
    // Destination stage: fragment shader (the fragment shader will read from them)
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         srcStage,
                         dstStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         static_cast<uint32_t>(barriers.size()),
                         barriers.data());
}

void CSJVulkanRenderer::generateYUVFrame(VkCommandBuffer commandBuffer, float time) {
     // 1. Update uniform buffer (time, etc.)
    YUVUniforms uniforms{};
    uniforms.time = time;
    uniforms.aspectRatio = (float)m_windowWidth / (float)m_windowHeight;
    memcpy(m_yuvUniformBufferMapped, &uniforms, sizeof(YUVUniforms));

    // 2. Bind compute pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_yuvComputePipeline);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_yuvComputePipelineLayout,
                            0,
                            1,
                            &m_yuvComputeDescriptorSet,
                            0,
                            nullptr);

    // 3. Dispatch
    uint32_t groupX = (m_swapchain_extent.width + 15) / 16;
    uint32_t groupY = (m_swapchain_extent.height + 15) / 16;
    vkCmdDispatch(commandBuffer, groupX, groupY, 1);

    // 4. Transition YUV images from GENERAL to SHADER_READ_ONLY for fragment shader
    TransitionYUVToSampling(commandBuffer);
}

void CSJVulkanRenderer::initYUVComputeComponents() {
     // Create YUV resources
    createyuvDescriptorPool();
    createYUVStorageImages();
    createYUVImageViews();
    createYUVSampler();
    createYUVUniformBuffer();

    // // Create compute pipeline
    createYUVComputeDescriptorSetLayout();
    createYUVComputePipelineLayout();
    createYUVComputePipeline();
    createYUVComputeDescriptorSet();

    // // Create graphics pipeline
    createYUVDescriptorSetLayout();
    createYUVPipelineLayout();
    createYUVPipeline();
    createYUVDescriptorSet();
}

void CSJVulkanRenderer::unInitYUVComputeComponents() {
     // Destroy graphics pipeline resources
    if (m_yuvPipeline) {
        vkDestroyPipeline(m_device, m_yuvPipeline, nullptr);
    }
    if (m_yuvPipelineLayout) {
        vkDestroyPipelineLayout(m_device, m_yuvPipelineLayout, nullptr);
    }

    if (m_yuvDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, m_yuvDescriptorSetLayout, nullptr);
    }

    // Destroy compute pipeline resources
    if (m_yuvComputePipeline) {
        vkDestroyPipeline(m_device, m_yuvComputePipeline, nullptr);
    }

    if (m_yuvComputePipelineLayout) {
        vkDestroyPipelineLayout(m_device, m_yuvComputePipelineLayout, nullptr);
    }

    if (m_yuvComputeDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, m_yuvComputeDescriptorSetLayout, nullptr);
    }

    // Destroy YUV resources
    if (m_yuvSampler) {
        vkDestroySampler(m_device, m_yuvSampler, nullptr);
    }

    if (m_yuvYImageView) {
        vkDestroyImageView(m_device, m_yuvYImageView, nullptr);
    }

    if (m_yuvUImageView) {
        vkDestroyImageView(m_device, m_yuvUImageView, nullptr);
    }

    if (m_yuvVImageView) {
        vkDestroyImageView(m_device, m_yuvVImageView, nullptr);
    }

    if (m_yuvYImage) {
        vkDestroyImage(m_device, m_yuvYImage, nullptr);
    }

    if (m_yuvUImage) {
        vkDestroyImage(m_device, m_yuvUImage, nullptr);
    }

    if (m_yuvVImage) {
        vkDestroyImage(m_device, m_yuvVImage, nullptr);
    }

    if (m_yuvYMemory) {
        vkFreeMemory(m_device, m_yuvYMemory, nullptr);
    }

    if (m_yuvUMemory) {
        vkFreeMemory(m_device, m_yuvUMemory, nullptr);
    }

    if (m_yuvVMemory) {
        vkFreeMemory(m_device, m_yuvVMemory, nullptr);
    }

    if (m_yuvUniformBuffer) {
        vkDestroyBuffer(m_device, m_yuvUniformBuffer, nullptr);
    }

    if (m_yuvUniformBufferMemory) {
        vkFreeMemory(m_device, m_yuvUniformBufferMemory, nullptr);
    }

    if (m_yuvDescirptorPool) {
        vkDestroyDescriptorPool(m_device, m_yuvDescirptorPool, nullptr);
    }
}

void CSJVulkanRenderer::createYUVStorageImages() {
    auto createImage = [&](uint32_t w, uint32_t h, VkFormat fmt, VkImage& img, VkDeviceMemory& mem) {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent = {w, h, 1};
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.format = fmt;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.samples = VK_SAMPLE_COUNT_1_BIT;

        vkCreateImage(m_device, &info, nullptr, &img);

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(m_device, img, &req);

        VkMemoryAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = req.size;

        // Find device-local memory type
        VkPhysicalDeviceMemoryProperties props;
        vkGetPhysicalDeviceMemoryProperties(m_physical_device, &props);
        for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
            if (req.memoryTypeBits & (1 << i)) {
                if (props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                    alloc.memoryTypeIndex = i;
                    break;
                }
            }
        }

        vkAllocateMemory(m_device, &alloc, nullptr, &mem);
        vkBindImageMemory(m_device, img, mem, 0);
    };

    auto extent = m_swapchain_extent;

    createImage(extent.width, extent.height, VK_FORMAT_R8_UNORM, m_yuvYImage, m_yuvYMemory);
    createImage(extent.width / 2, extent.height / 2, VK_FORMAT_R8_UNORM, m_yuvUImage, m_yuvUMemory);
    createImage(extent.width / 2, extent.height / 2, VK_FORMAT_R8_UNORM, m_yuvVImage, m_yuvVMemory);
}

void CSJVulkanRenderer::createYUVImageViews() {
    auto createView = [&](VkImage img, VkFormat fmt, VkImageView& view) {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = img;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = fmt;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        vkCreateImageView(m_device, &info, nullptr, &view);
    };

    createView(m_yuvYImage, VK_FORMAT_R8_UNORM, m_yuvYImageView);
    createView(m_yuvUImage, VK_FORMAT_R8_UNORM, m_yuvUImageView);
    createView(m_yuvVImage, VK_FORMAT_R8_UNORM, m_yuvVImageView);
}

void CSJVulkanRenderer::createYUVDescriptorSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 3> bindings{};

    // Binding 0: Y texture
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Binding 1: U texture
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Binding 2: V texture
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_yuvDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create YUV graphics descriptor set layout!");
    }
}

void CSJVulkanRenderer::createYUVPipelineLayout() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_yuvDescriptorSetLayout;

    if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_yuvPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create YUV graphics pipeline layout!");
    }

}

void CSJVulkanRenderer::createYUVPipeline() {
        // 1. Load shaders
    auto vertShaderCode = readFile("resources/shaders/yuv_vert.spv");
    auto fragShaderCode = readFile("resources/shaders/yuv_frag.spv");

    VkShaderModule vertModule = createShaderModule(vertShaderCode);
    VkShaderModule fragModule = createShaderModule(fragShaderCode);

    // 2. Shader stages
    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

    // 3. Vertex input (no vertex buffers)
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;

    // 4. Input assembly (triangle list)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 5. Viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_windowWidth);
    viewport.height = static_cast<float>(m_windowHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(m_windowWidth), static_cast<uint32_t>(m_windowHeight)};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // 6. Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    // 7. Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;

    // 8. Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // 9. Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_yuvPipelineLayout;
    pipelineInfo.renderPass = m_render_pass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_yuvPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create YUV graphics pipeline!");
    }

    // 10. Cleanup shader modules
    vkDestroyShaderModule(m_device, vertModule, nullptr);
    vkDestroyShaderModule(m_device, fragModule, nullptr);

    std::cout << "[VulkanRenderer] YUV graphics pipeline created." << std::endl;
}

void CSJVulkanRenderer::createYUVDescriptorSet() {
    // 1. Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_yuvDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_yuvDescriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_yuvDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate YUV graphics descriptor set!");
    }

    // 2. Create image views for YUV storage images (if not already done)
    // These views are used for sampling (not storage)
    // CreateYUVImageViews();

    // 3. Update descriptor set
    std::array<VkDescriptorImageInfo, 3> imageInfos{};
    imageInfos[0].sampler = m_yuvSampler;  // Create a sampler for YUV textures
    imageInfos[0].imageView = m_yuvYImageView;
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageInfos[1].sampler = m_yuvSampler;
    imageInfos[1].imageView = m_yuvUImageView;
    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageInfos[2].sampler = m_yuvSampler;
    imageInfos[2].imageView = m_yuvVImageView;
    imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::array<VkWriteDescriptorSet, 3> writes{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = m_yuvDescriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].pImageInfo = &imageInfos[0];

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = m_yuvDescriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &imageInfos[1];

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = m_yuvDescriptorSet;
    writes[2].dstBinding = 2;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].pImageInfo = &imageInfos[2];

    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void CSJVulkanRenderer::createYUVSampler() {
    VkSamplerCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(m_device, &info, nullptr, &m_yuvSampler);
}

void CSJVulkanRenderer::createYUVUniformBuffer() {
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = sizeof(YUVUniforms);
    info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    vkCreateBuffer(m_device, &info, nullptr, &m_yuvUniformBuffer);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(m_device, m_yuvUniformBuffer, &req);

    VkMemoryAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = req.size;

    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if (req.memoryTypeBits & (1 << i)) {
            if (props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                alloc.memoryTypeIndex = i;
                break;
            }
        }
    }

    vkAllocateMemory(m_device, &alloc, nullptr, &m_yuvUniformBufferMemory);
    vkBindBufferMemory(m_device, m_yuvUniformBuffer, m_yuvUniformBufferMemory, 0);
    vkMapMemory(m_device, m_yuvUniformBufferMemory, 0, sizeof(YUVUniforms), 0, &m_yuvUniformBufferMapped);
}

} // namespace csjrhi
