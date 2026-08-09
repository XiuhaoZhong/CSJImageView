#include "CSJVulkanHelper.h"

#include <fstream>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

namespace csjrhi {

CSJVulkanHelper::CSJVulkanHelper(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_queue(queue) {
    // Create a command pool for immediate commands
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = 0; // Assuming graphics queue family index 0

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool for VulkanHelper!");
    }
}

CSJVulkanHelper::~CSJVulkanHelper() {
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}

CSJSpTexture CSJVulkanHelper::CreateTexture2D(uint32_t width, 
                                              uint32_t height, 
                                              CSJPixelFormat format, 
                                              const void *data, 
                                              size_t dataSize, 
                                              bool generateMipmaps) {
    VkFormat vkFormat = ToVkFormat(format);

    // --- 1. Create Image ---
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vkFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage image;
    if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image!");
    }

    // --- 2. Allocate Memory ---
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    // Find memory type that is device-local and supports the image
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (memRequirements.memoryTypeBits & (1 << i)) {
            if (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                allocInfo.memoryTypeIndex = i;
                break;
            }
        }
    }

    VkDeviceMemory imageMemory;
    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        vkDestroyImage(m_device, image, nullptr);
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, image, imageMemory, 0);

    // --- 3. Transition Layout to Transfer Dst ---
    TransitionImageLayout(image, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // --- 4. Upload Data (if provided) ---
    if (data && dataSize > 0) {
        // Create staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = dataSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer);

        VkMemoryRequirements stagingMemReqs;
        vkGetBufferMemoryRequirements(m_device, stagingBuffer, &stagingMemReqs);

        VkMemoryAllocateInfo stagingAllocInfo{};
        stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        stagingAllocInfo.allocationSize = stagingMemReqs.size;

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (stagingMemReqs.memoryTypeBits & (1 << i)) {
                if (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                    stagingAllocInfo.memoryTypeIndex = i;
                    break;
                }
            }
        }

        vkAllocateMemory(m_device, &stagingAllocInfo, nullptr, &stagingMemory);
        vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0);

        // Copy data to staging buffer
        void* mappedData;
        vkMapMemory(m_device, stagingMemory, 0, dataSize, 0, &mappedData);
        memcpy(mappedData, data, dataSize);
        vkUnmapMemory(m_device, stagingMemory);

        // Copy from staging buffer to image
        CopyBufferToImage(stagingBuffer, image, width, height);

        // Clean up staging resources
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingMemory, nullptr);
    }

    // --- 5. Transition Layout to Shader Read Only ---
    TransitionImageLayout(image, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // --- 6. Create Image View ---
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = vkFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        vkDestroyImage(m_device, image, nullptr);
        vkFreeMemory(m_device, imageMemory, nullptr);
        throw std::runtime_error("failed to create texture image view!");
    }

    // --- 7. Create Sampler ---
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.mipLodBias = 0.0f;

    VkSampler sampler;
    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        vkDestroyImageView(m_device, imageView, nullptr);
        vkDestroyImage(m_device, image, nullptr);
        vkFreeMemory(m_device, imageMemory, nullptr);
        throw std::runtime_error("failed to create texture sampler!");
    }

    // --- 8. Wrap in TextureData and return ---
    auto textureData = std::make_unique<TextureData>();
    textureData->device = m_device;
    textureData->image = image;
    textureData->memory = imageMemory;
    textureData->view = imageView;
    textureData->sampler = sampler;
    textureData->width = width;
    textureData->height = height;
    textureData->format = format;
    textureData->mipLevels = 1;

    return textureData;
}

CSJSpTexture CSJVulkanHelper::CreateTextureFromFile(const std::string &filePath) {
    if (filePath.empty()) {
        return nullptr;
    }

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4; // force using rgba.
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    CSJSpTexture textureData = CreateTexture2D(texWidth, 
                                               texHeight, 
                                               CSJPixelFormat::CSJPixelFormat_R8G8B8A8_SRGB, 
                                               pixels, 
                                               imageSize, 
                                               false);

    return textureData;
    /**  For now, just a placeholder — you can implement this later
     * using stb_image or similar library.
     * Example: load image, then call CreateTexture2D.
     */
    //throw std::runtime_error("CreateTextureFromFile not implemented yet.");
}

void CSJVulkanHelper::UpdateTexture(ICSJTexture *texture, const void *data, size_t dataSize) {
    /**  For now, just a placeholder.
     * You can implement partial updates here.
     * This would involve creating a staging buffer and copying.
     */
    throw std::runtime_error("UpdateTexture not yet implemented.");
}

void CSJVulkanHelper::DestroyTexture(ICSJTexture *texture) {
    if (!texture) {
        return;
    }

    TextureData* texData = static_cast<TextureData*>(texture);

    // Wait for GPU to finish
    vkDeviceWaitIdle(m_device);

    if (texData->sampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_device, texData->sampler, nullptr);
    }

    if (texData->view != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, texData->view, nullptr);
    }

    if (texData->image != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, texData->image, nullptr);
    }

    if (texData->memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, texData->memory, nullptr);
    }

    delete texData;
}

CSJSpBuffer CSJVulkanHelper::CreateBuffer(size_t size, CSJBufferUsage usage, const void *data) {
    VkBufferUsageFlags vkUsage = ToVkBufferUsage(usage);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = vkUsage;

    VkBuffer buffer;
    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    bool foundMemory = false;
    VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // If the buffer is used for staging, we need CPU-visible memory
    if (usage == CSJBufferUsage::CSJBufferUsage_Staging) {
        requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (memRequirements.memoryTypeBits & (1 << i)) {
            if ((memProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) {
                allocInfo.memoryTypeIndex = i;
                foundMemory = true;
                break;
            }
        }
    }

    if (!foundMemory) {
        vkDestroyBuffer(m_device, buffer, nullptr);
        throw std::runtime_error("failed to find suitable memory type for buffer!");
    }

    VkDeviceMemory memory;
    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        vkDestroyBuffer(m_device, buffer, nullptr);
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_device, buffer, memory, 0);

    // If data is provided, upload it
    if (data && size > 0 && usage == CSJBufferUsage::CSJBufferUsage_Staging) {
        void* mappedData;
        vkMapMemory(m_device, memory, 0, size, 0, &mappedData);
        memcpy(mappedData, data, size);
        vkUnmapMemory(m_device, memory);
    }

    auto bufferData = std::make_unique<BufferData>();
    bufferData->buffer = buffer;
    bufferData->memory = memory;
    bufferData->size = size;
    bufferData->usage = usage;

    return bufferData;
}

void CSJVulkanHelper::UpdateBuffer(ICSJBuffer *buffer, const void *data, size_t dataSize, size_t offset) {
    if (!buffer || !data) {
        return;
    }

    BufferData* bufData = static_cast<BufferData*>(buffer);

    void* mappedData;
    vkMapMemory(m_device, bufData->memory, offset, dataSize, 0, &mappedData);
    memcpy(mappedData, data, dataSize);
    vkUnmapMemory(m_device, bufData->memory);
}

void CSJVulkanHelper::DestroyBuffer(ICSJBuffer *buffer) {
    if (!buffer) {
        return;
    }

    BufferData* bufData = static_cast<BufferData*>(buffer);

    vkDestroyBuffer(m_device, bufData->buffer, nullptr);
    vkFreeMemory(m_device, bufData->memory, nullptr);

    delete bufData;
}

void CSJVulkanHelper::ExecuteImmediate(const std::function<void(void *)> &commands) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    // Execute the user-provided commands
    if (commands) {
        commands(reinterpret_cast<void*>(commandBuffer));
    }

    EndSingleTimeCommands(commandBuffer);
}

std::string CSJVulkanHelper::GetBackendName() const {
    return "Vulkan";
}

void *CSJVulkanHelper::GetDeviceHandle() const {
     return reinterpret_cast<void*>(m_device);
}

VkFormat CSJVulkanHelper::ToVkFormat(CSJPixelFormat format) const {
    switch (format) {
        case CSJPixelFormat::CSJPixelFormat_R8G8B8A8_SRGB:   
            return VK_FORMAT_R8G8B8A8_SRGB;
        case CSJPixelFormat::CSJPixelFormat_R8G8B8A8_UNORM:  
            return VK_FORMAT_R8G8B8A8_UNORM;
        case CSJPixelFormat::CSJPixelFormat_R8G8B8_UNORM:    
            return VK_FORMAT_R8G8B8_UNORM;
        case CSJPixelFormat::CSJPixelFormat_R8_UNORM:        
            return VK_FORMAT_R8_UNORM;
        case CSJPixelFormat::CSJPixelFormat_R16G16B16A16_SFLOAT: 
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case CSJPixelFormat::CSJPixelFormat_R32G32B32A32_SFLOAT: 
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case CSJPixelFormat::CSJPixelFormat_D32_SFLOAT_S8_UINT:  
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case CSJPixelFormat::CSJPixelFormat_D32_SFLOAT:          
            return VK_FORMAT_D32_SFLOAT;
        case CSJPixelFormat::CSJPixelFormat_D24_UNORM_S8_UINT:   
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case CSJPixelFormat::CSJPixelFormat_D16_UNORM:           
            return VK_FORMAT_D16_UNORM;
        case CSJPixelFormat::CSJPixelFormat_BC1_RGB_UNORM:       
            return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case CSJPixelFormat::CSJPixelFormat_BC1_RGBA_UNORM:      
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case CSJPixelFormat::CSJPixelFormat_BC3_UNORM:           
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case CSJPixelFormat::CSJPixelFormat_BC7_UNORM:           
            return VK_FORMAT_BC7_UNORM_BLOCK;
        default: 
            return VK_FORMAT_UNDEFINED;
    }
}

VkBufferUsageFlags CSJVulkanHelper::ToVkBufferUsage(CSJBufferUsage usage) const {
    VkBufferUsageFlags flags = 0;
    if (usage == CSJBufferUsage::CSJBufferUsage_Vertex) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    
    if (usage == CSJBufferUsage::CSJBufferUsage_Index) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if (usage == CSJBufferUsage::CSJBufferUsage_Uniform) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (usage == CSJBufferUsage::CSJBufferUsage_Storage) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if (usage == CSJBufferUsage::CSJBufferUsage_Staging) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    if (usage == CSJBufferUsage::CSJBufferUsage_Transfer) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if (usage == CSJBufferUsage::CSJBufferUsage_Indirect) {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }

    return flags;
}

VkCommandBuffer CSJVulkanHelper::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CSJVulkanHelper::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void CSJVulkanHelper::TransitionImageLayout(VkImage image, 
                                            VkFormat format, 
                                            VkImageLayout oldLayout, 
                                            VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

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

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        // Default: just a layout transition
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage,
                         destinationStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

void CSJVulkanHelper::CopyBufferToImage(VkBuffer buffer, 
                                        VkImage image, 
                                        uint32_t width, 
                                        uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    EndSingleTimeCommands(commandBuffer);
}

}
