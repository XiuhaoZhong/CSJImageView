#pragma once

#include "ICSJRenderer.h"

#include <vector>
#include <optional>
#include <memory>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 
#endif
#include <vulkan/vulkan.h>

#include "CSJVulkanHelper.h"
#include "ICSJRenderable.h"

namespace csjrhi {

enum class CSJRenderType : uint8_t {
    Common = 0,
    Renderable
};

using CSJSpVulkanHelper = std::shared_ptr<CSJVulkanHelper>;

struct YUVUniforms {
    float time;         // For animation
    float aspectRatio;  // For aspect-correct patterns
    float padding[2];   // Align to 16 bytes (optional, but good practice)
};

struct QueueFamilyIndices {
    std::optional<uint32_t> m_graphics_family;
    std::optional<uint32_t> m_present_family;

    bool isComplete() {
        return m_graphics_family.has_value() && m_present_family.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};



class CSJVulkanRenderer : public ICSJRenderer {
public:
    CSJVulkanRenderer() = default;
    ~CSJVulkanRenderer();

    bool Init(void* windowHandle, int width, int height) override;
    void Shutdown() override;
    void Resize(int width, int height) override;
    void Render(float timeStamp) override;
    void WaitIdle() override;
    uint32_t CreateTexture(int width, int height, int format, const void* data) override;
    void DestroyTexture(uint32_t textureId) override;
    void UpdateTexture(uint32_t textureId, const void* data) override;
    std::string GetBackendName() const override;
    float GetLastFrameTime() const override;
    const CSJRendererCapabilities& GetCapabilities() const override;

    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createBuffer(VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDevice getDevice() const {
        return m_device;
    }

    VkPhysicalDevice getPhysicalDevice() const {
        return m_physical_device;
    }

    VkRenderPass getRenderPass() const {
        return m_render_pass;
    }

    VkCommandBuffer getCommandBuffer() const {
        return m_command_buffers[m_current_frame];
    }

    VkDescriptorPool getDescriptorPool() const {
        return m_descripotrPoolForRenderables;
    }

    VkExtent2D getSwapchainExtent() const {
        return m_swapchain_extent;
    }

    VkSurfaceFormatKHR getSurfaceFormat() const {
        return m_surfaceFormat;
    }

    CSJSpVulkanHelper getHelper() const {
        return m_pHelper;
    }

    int getWindowWidth() const {
        return m_windowWidth;
    }

    int getWindowHeight() const {
        return m_windowHeight;
    }

protected:
    void initVulkan();

    std::array<int, 2> getCurrentWindowSize();

    void cleanup();
    void cleanupSwapChain();

    void createInstance();

    bool checkValidationLayerSupport();
    std::vector<const char *> getRequiredExtensions();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void setupDebugMessenger();

    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();

    void createVulkanHelper();

    void createSurface();

    void createSwapChain();
    void recreateSwapChain();

    void createImageViews();

    void createRenderPass();

    void createFrameBuffers();

    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void createDescriptorPool();

    void drawFrame();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
     * The following functions and memebers are for supporting renderable rendering.
     */

    void initForRenderables();

    void createDescriptorPoolForRenderables();

    void createOffscreenSampler();
    void createOffscreenResources();
    void destroyOffscreenResources();

    void reCreateOffscreenResources();

    void TransitionOffscreenToColorAttachment(VkCommandBuffer cmd);
    void TransitionOffscreenToShaderReadOnly(VkCommandBuffer cmd);

private:
    CSJRenderType m_renderType = CSJRenderType::Common;
    void *m_pWindow;
    int   m_windowWidth = 0;
    int   m_windowHeight = 0;
    bool  m_bNeedRecreateSwapChain = false;
    VkImageLayout m_offscreenLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    CSJSpVulkanHelper m_pHelper = nullptr; 
    VkInstance  m_VkInstance{nullptr};
    
    VkDevice         m_device;
    VkQueue          m_graphics_queue;
    VkQueue          m_present_queue;
    VkSurfaceKHR     m_surface;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;

    VkSurfaceFormatKHR   m_surfaceFormat;
    VkSwapchainKHR       m_swapchain{nullptr};
    std::vector<VkImage> m_swapchain_images;
    VkFormat             m_swapchain_image_format;
    VkExtent2D           m_swapchain_extent;
    std::vector<VkImageView> m_swapchain_imageViews;
    std::vector<VkFramebuffer> m_swapchain_frame_buffers;

    VkRenderPass          m_render_pass;
    VkCommandPool         m_command_pool;
    std::vector<VkCommandBuffer> m_command_buffers;

    VkImage         m_offscreenImage;
    VkDeviceMemory  m_offscreenMemory;
    VkImageView     m_offscreenImageView;
    VkFramebuffer   m_offscreenFramebuffer;
    VkSampler       m_offscreenSampler;

    /* This descriptor pool is for image renderer and yuv renderer. */
    VkDescriptorPool m_descripotrPoolForRenderables = VK_NULL_HANDLE;
    /* This descriptor pool is for common rendering in the future. */
    VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;

    std::vector<VkSemaphore> m_image_available_semas;
    std::vector<VkSemaphore> m_render_finish_semas;
    std::vector<VkFence>     m_in_flight_fences;
    uint32_t                 m_current_frame = 0;
    bool                     m_bFrameBufferResize = false;

    bool m_enable_validation_Layers{ true };
    bool m_enable_debug_utils_label{ true };
    const std::vector<const char *> m_validation_layers{"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> m_device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    CSJRendererCapabilities m_renderCaps;

    CSJSpRenderable m_postProcessRenderable;
    std::vector<CSJSpRenderable> m_renderables;

};

} // namespace csjrhi