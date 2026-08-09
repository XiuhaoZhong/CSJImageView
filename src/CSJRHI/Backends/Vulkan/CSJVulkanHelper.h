#pragma once

#include "ICSJGraphicsHelper.h"

#include <vulkan/vulkan.h>

namespace csjrhi {

class CSJVulkanHelper : public ICSJGraphicsHelper {
public:
    CSJVulkanHelper(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue);
    ~CSJVulkanHelper();
    
    // --- Texture ---
    CSJSpTexture CreateTexture2D(uint32_t width,
                                 uint32_t height,
                                 CSJPixelFormat format,           // Abstract format (RGBA8, YUV, etc.)
                                 const void* data,
                                 size_t dataSize,
                                 bool generateMipmaps = false) override;

    CSJSpTexture CreateTextureFromFile(const std::string& filePath) override;

    void UpdateTexture(ICSJTexture* texture,
                       const void* data,
                       size_t dataSize) override;

    void DestroyTexture(ICSJTexture* texture) override;

    // --- Buffer ---
    CSJSpBuffer CreateBuffer(size_t size,
                             CSJBufferUsage usage,            // Vertex, Index, Uniform, Staging
                             const void* data = nullptr) override;

    void UpdateBuffer(ICSJBuffer* buffer,
                      const void* data,
                      size_t dataSize,
                      size_t offset = 0) override;

    void DestroyBuffer(ICSJBuffer* buffer) override;

    // --- Command Buffer (Helper) ---
    void ExecuteImmediate(const std::function<void(void*)>& commands) override;  // For single-time commands (e.g., upload data)

// --- Info ---
    std::string GetBackendName() const override;
    void* GetDeviceHandle() const override;

protected:
    VkFormat ToVkFormat(CSJPixelFormat format) const;
    VkBufferUsageFlags ToVkBufferUsage(CSJBufferUsage usage) const;
    // VkShaderStageFlagBits ToVkShaderStage(CSJShaderStage stage) const;
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    void TransitionImageLayout(
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );

    void CopyBufferToImage(
        VkBuffer buffer,
        VkImage image,
        uint32_t width,
        uint32_t height
    );

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
        uint32_t width        = 0;
        uint32_t height       = 0;
        CSJPixelFormat format = CSJPixelFormat::CSJPixelFormat_NONE;
        uint32_t mipLevels    = 1;

        // ITexture interface
        void* GetNativeHandle() override { return reinterpret_cast<void*>(view); }
        void* GetSampler() override { return reinterpret_cast<void*>(sampler);}
        uint32_t GetWidth() const override { return width; }
        uint32_t GetHeight() const override { return height; }
        uint32_t GetMipLevels() const override { return mipLevels; }
        CSJPixelFormat GetFormat() const override { return format; }

        ~TextureData() {
            if (!device) {
                return ;
            }

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

            device = VK_NULL_HANDLE;
        }
    };

    struct BufferData : public ICSJBuffer {
        VkBuffer buffer       = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        size_t size           = 0;
        CSJBufferUsage usage  = CSJBufferUsage::CSJBufferUsage_None;

        void* GetNativeHandle() override { return reinterpret_cast<void*>(buffer); }
        size_t GetSize() const override { return size; }
        CSJBufferUsage GetUsage() const override { return usage; }
    };

    // struct ShaderData : public ICSJShader {
    //     VkShaderModule module = VK_NULL_HANDLE;
    //     CSJShaderStage stage = CSJShaderStage::CSJShaderStage_Unknown;
    //     std::string name;

    //     void* GetNativeHandle() override { return reinterpret_cast<void*>(module); }
    //     const std::string& GetName() const override { return name; }
    //     CSJShaderStage GetStage() const override { return stage; }
    // };

private:
    VkDevice         m_device;
    VkPhysicalDevice m_physicalDevice;
    VkQueue          m_queue;
    VkCommandPool    m_commandPool;
};

}