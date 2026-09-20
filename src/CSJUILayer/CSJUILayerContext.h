#pragma once

#include <cstdint>

namespace csjuilayer {

enum class CSJBackendType {
    Vulkan,
    DX11,
    DX12,
    Metal
};

struct CSJUILayerContext {
    CSJBackendType backendType;

    // Platform handles (cast to the correct type in the backend)
    void* instance = nullptr;          // VkInstance, ...
    void* physicalDevice = nullptr;    // VkPhysicalDevice, ...
    void* device = nullptr;            // VkDevice, ID3D11Device*, MTLDevice
    void* queue = nullptr;             // VkQueue, ID3D11DeviceContext*, MTLCommandQueue
    void* renderPass = nullptr;        // VkRenderPass, ...
    void* descriptorPool = nullptr;    // VkDescriptorPool, ...

    // Queue family index (Vulkan-specific, but harmless for other APIs)
    uint32_t queueFamilyIndex = 0;

    // Window size (for ImGui display size)
    uint32_t width = 0;
    uint32_t height = 0;
};

} // namespace csjuilayer