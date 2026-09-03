#pragma once

#include "ICSJGraphicsHelper.h"

#include <vulkan/vulkan.h>

namespace csjrhi {

struct CSJVulkanHelperContext {
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    bool validate() {
        return (device != VK_NULL_HANDLE &&
                    physical_device != VK_NULL_HANDLE &&
                    commandPool != VK_NULL_HANDLE &&
                    commandBuffer != VK_NULL_HANDLE &&
                    queue != VK_NULL_HANDLE);
    }
};

class CSJVulkanHelper {
public:
    CSJVulkanHelper() = default;
    ~CSJVulkanHelper() = default;
    
    // --- Texture ---
    static CSJSpTexture CreateTexture2D(CSJVulkanHelperContext *context,
                                             uint32_t width,
                                             uint32_t height,
                                             VkFormat format,
                                             const void *data,
                                             size_t dataSize);

    static void WaitForTextureUpload(CSJSpTexture& info);
    static bool IsTextureUploadComplete(CSJSpTexture& info);
    static void DestroyTexture(CSJSpTexture& info);

    static CSJSpTexture CreateTextureFromFile(CSJVulkanHelperContext *context,
                                              const std::string& filePath);

    static void UpdateTexture(ICSJTexture* texture,
                              const void* data,
                              size_t dataSize);

    static void DestroyTexture(VkDevice device, ICSJTexture* texture);

    // --- Buffer ---
    static CSJSpBuffer CreateBuffer(VkDevice device, 
                                    VkPhysicalDevice physical_device,
                                    size_t size,
                                    CSJBufferUsage usage, // Vertex, Index, Uniform, Staging
                                    const void* data = nullptr);

    static void UpdateBuffer(VkDevice device,
                             ICSJBuffer* buffer,
                             const void* data,
                             size_t dataSize,
                             size_t offset = 0) ;

    static void DestroyBuffer(VkDevice device, ICSJBuffer* buffer);

    // --- Info ---
    static std::string GetBackendName();

protected:
    static VkFormat ToVkFormat(CSJPixelFormat format);
    static VkBufferUsageFlags ToVkBufferUsage(CSJBufferUsage usage);
    static VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commadPool);
    static void EndSingleTimeCommands(VkDevice device,
                                      VkCommandBuffer commandBuffer,
                                      VkCommandPool commadPool,
                                      VkQueue queue);

    static void CopyBufferToImage(VkDevice device,
                                  VkCommandPool commandPool,
                                  VkQueue queue,VkBuffer buffer,
                                  VkImage image,
                                  uint32_t width,
                                  uint32_t height);

private:

    // ------------------------------------------------------------------------
    // Internal Resource Types
    // ------------------------------------------------------------------------
    struct TextureData : public ICSJTexture {
        VkDevice device       = VK_NULL_HANDLE;
        VkImage image         = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE; 
        VkImageView view      = VK_NULL_HANDLE;
        VkSampler sampler     = VK_NULL_HANDLE;
        VkBuffer  stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

        uint32_t width        = 0;
        uint32_t height       = 0;
        CSJPixelFormat format = CSJPixelFormat::CSJPixelFormat_NONE;
        uint32_t mipLevels    = 1;
        VkFence fence = VK_NULL_HANDLE;
        bool isComplete = false;

        // ITexture interface
        void* GetNativeHandle()  { return reinterpret_cast<void*>(view); }
        void* GetSampler()  { return reinterpret_cast<void*>(sampler);}
        uint32_t GetWidth() const  { return width; }
        uint32_t GetHeight() const  { return height; }
        uint32_t GetMipLevels() const  { return mipLevels; }
        CSJPixelFormat GetFormat() const  { return format; }

        ~TextureData() {
            if (sampler) {
                vkDestroySampler(device, sampler, nullptr);
            }
            
            if (view) {
                vkDestroyImageView(device, view, nullptr);
            }
            
            if (image) {
                vkDestroyImage(device, image, nullptr);    
            }
            
            if (memory) {
                vkFreeMemory(device, memory, nullptr);
            }

            if (device) {
                device = VK_NULL_HANDLE;
            }
        }
    };

    struct BufferData : public ICSJBuffer {
        VkBuffer buffer       = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        size_t size           = 0;
        CSJBufferUsage usage  = CSJBufferUsage::CSJBufferUsage_None;

        void* GetNativeHandle()  { return reinterpret_cast<void*>(buffer); }
        size_t GetSize() const  { return size; }
        CSJBufferUsage GetUsage() const  { return usage; }
    };
};

}