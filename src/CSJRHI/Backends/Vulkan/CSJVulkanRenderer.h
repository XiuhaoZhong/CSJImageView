#pragma once

#include "ICSJRenderer.h"

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace csjrhi {

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
    void Render() override;
    void WaitIdle() override;
    uint32_t CreateTexture(int width, int height, int format, const void* data) override;
    void DestroyTexture(uint32_t textureId) override;
    void UpdateTexture(uint32_t textureId, const void* data) override;
    std::string GetBackendName() const override;
    float GetLastFrameTime() const override;
    const CSJRendererCapabilities& GetCapabilities() const override;

protected:
    void initVulkan();

    void resizeFramebuffer(int width, int height);

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

    void createSurface();

    void createSwapChain();
    void recreateSwapChain();

    void createImageViews();

    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createGraphicsPipeline();

    void createRenderPass();

    void createFrameBuffers();

    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();

    void createUniformBuffers();
    void createVertexBuffer();
    void createIndexBuffer();

    void createTextureImage();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createTextureImageView();
    void createTextureImageSampler();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkImageView createImageView(VkImage image, VkFormat format);
    void createBuffer(VkDeviceSize size, 
                      VkBufferUsageFlags usage, 
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, 
                      VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void updateUniformBuffer(uint32_t currentImage);
    void drawFrame();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    GLFWwindow *m_pWindow;
    VkInstance  m_VkInstance{nullptr};
    
    VkDevice         m_device;
    VkQueue          m_graphics_queue;
    VkQueue          m_present_queue;
    VkSurfaceKHR     m_surface;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;

    VkSwapchainKHR       m_swapchain{nullptr};
    std::vector<VkImage> m_swapchain_images;
    VkFormat             m_swapchain_image_format;
    VkExtent2D           m_swapchain_extent;
    std::vector<VkImageView> m_swapchain_imageViews;
    std::vector<VkFramebuffer> m_swapchain_frame_buffers;

    VkRenderPass          m_render_pass;
    VkDescriptorSetLayout m_descriptorset_layout;
    VkPipelineLayout      m_pipeline_layout;
    VkPipeline            m_graphics_pipeline;
    VkCommandPool         m_command_pool;
    //VkCommandBuffer  m_command_buffer;
    std::vector<VkCommandBuffer> m_command_buffers;

    VkDescriptorPool m_descriptor_pool;
    std::vector<VkDescriptorSet> m_descriptor_sets;

    VkBuffer         m_vertex_buffer;
    VkDeviceMemory   m_vertex_buffer_memory;

    VkBuffer         m_index_buffer;
    VkDeviceMemory   m_index_buffer_memory;

    VkImage          m_texture_image;
    VkDeviceMemory   m_texture_image_momery;
    VkImageView      m_texture_imageview;
    VkSampler        m_texture_image_sampler;

    std::vector<VkBuffer>       m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_buffer_memories;
    std::vector<void *>         m_uniform_buffer_mappeds;

    //VkSemaphore      m_image_available_sema;
    //VkSemaphore      m_render_finish_sema;
    //VkFence          m_in_flight_fence;

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

};

} // namespace csjrhi