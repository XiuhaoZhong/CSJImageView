#include "CSJVulkanHelper.h"

#include <fstream>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

namespace csjrhi {

CSJSpTexture CSJVulkanHelper::CreateTexture2D(CSJVulkanHelperContext *context,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   VkFormat format,
                                                   const void *data,
                                                   size_t dataSize) {
    if (!context || !context->validate()) {
        return nullptr;
    }
    // ──────────────────────────────────────────────
    // 1. Create the image
    // ──────────────────────────────────────────────

    auto textureData = std::make_shared<TextureData>();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(context->device, &imageInfo, nullptr, &textureData->image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // ──────────────────────────────────────────────
    // 2. Allocate image memory
    // ──────────────────────────────────────────────
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(context->device, textureData->image, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(context->physical_device, &memProps);

    bool found = false;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if (memReqs.memoryTypeBits & (1 << i)) {
            if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                allocInfo.memoryTypeIndex = i;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        vkDestroyImage(context->device, textureData->image, nullptr);
        throw std::runtime_error("failed to find suitable memory type for image!");
    }

    if (vkAllocateMemory(context->device, &allocInfo, nullptr, &textureData->memory) != VK_SUCCESS) {
        vkDestroyImage(context->device,  textureData->image, nullptr);
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(context->device,  textureData->image, textureData->memory, 0);

    // ──────────────────────────────────────────────
    // 3. Create staging buffer
    // ──────────────────────────────────────────────
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = dataSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer stagingBuffer;
    if (vkCreateBuffer(context->device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        vkDestroyImage(context->device, textureData->image, nullptr);
        vkFreeMemory(context->device, textureData->memory, nullptr);
        throw std::runtime_error("failed to create staging buffer!");
    }

    VkMemoryRequirements stagingMemReqs;
    vkGetBufferMemoryRequirements(context->device, stagingBuffer, &stagingMemReqs);

    VkMemoryAllocateInfo stagingAllocInfo{};
    stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    stagingAllocInfo.allocationSize = stagingMemReqs.size;

    found = false;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if (stagingMemReqs.memoryTypeBits & (1 << i)) {
            if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                stagingAllocInfo.memoryTypeIndex = i;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        vkDestroyBuffer(context->device, stagingBuffer, nullptr);
        vkDestroyImage(context->device, textureData->image, nullptr);
        vkFreeMemory(context->device, textureData->memory, nullptr);
        throw std::runtime_error("failed to find staging memory type!");
    }

    VkDeviceMemory stagingMemory;
    if (vkAllocateMemory(context->device, &stagingAllocInfo, nullptr, &stagingMemory) != VK_SUCCESS) {
        vkDestroyBuffer(context->device, stagingBuffer, nullptr);
        vkDestroyImage(context->device, textureData->image, nullptr);
        vkFreeMemory(context->device, textureData->memory, nullptr);
        throw std::runtime_error("failed to allocate staging memory!");
    }

    vkBindBufferMemory(context->device, stagingBuffer, stagingMemory, 0);

    // ──────────────────────────────────────────────
    // 4. Copy data to staging buffer
    // ──────────────────────────────────────────────
    void* mappedData;
    vkMapMemory(context->device, stagingMemory, 0, dataSize, 0, &mappedData);
    memcpy(mappedData, data, dataSize);
    vkUnmapMemory(context->device, stagingMemory);

    // ──────────────────────────────────────────────
    // 5. Allocate (or reuse) command buffer
    // ──────────────────────────────────────────────
    // Reset the command buffer (reuse)
    vkResetCommandBuffer(context->commandBuffer, 0);

    // ──────────────────────────────────────────────
    // 6. Begin recording
    // ──────────────────────────────────────────────
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(context->commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin command buffer!");
    }

    // ──────────────────────────────────────────────
    // 7. Transition image to TRANSFER_DST_OPTIMAL
    // ──────────────────────────────────────────────
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureData->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(context->commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &barrier);

    // ──────────────────────────────────────────────
    // 8. Copy buffer to image
    // ──────────────────────────────────────────────
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

    vkCmdCopyBufferToImage(context->commandBuffer, stagingBuffer, textureData->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);

    // ──────────────────────────────────────────────
    // 9. Transition image to SHADER_READ_ONLY_OPTIMAL
    // ──────────────────────────────────────────────
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(context->commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &barrier);

    // ──────────────────────────────────────────────
    // 10. End command buffer
    // ──────────────────────────────────────────────
    if (vkEndCommandBuffer(context->commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to end command buffer!");
    }

    // ──────────────────────────────────────────────
    // 11. Create fence for this upload
    // ──────────────────────────────────────────────
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

    if (vkCreateFence(context->device, &fenceInfo, nullptr, &textureData->fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

    // ──────────────────────────────────────────────
    // 12. Submit command buffer with fence
    // ──────────────────────────────────────────────
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context->commandBuffer;

    if (vkQueueSubmit(context->queue, 1, &submitInfo, textureData->fence) != VK_SUCCESS) {
        vkDestroyFence(context->device, textureData->fence, nullptr);
        throw std::runtime_error("failed to submit command buffer!");
    }

    // ──────────────────────────────────────────────
    // 13. Clean up staging buffer (immediate)
    // ──────────────────────────────────────────────
    // vkDestroyBuffer(context->device, stagingBuffer, nullptr);
    // vkFreeMemory(context->device, stagingMemory, nullptr);

    textureData->stagingBuffer = stagingBuffer;
    textureData->stagingMemory = stagingMemory;

    // ──────────────────────────────────────────────
    // 14. Create image view (immediate)
    // ──────────────────────────────────────────────
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureData->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(context->device, &viewInfo, nullptr, &textureData->view) != VK_SUCCESS) {
        vkDestroyFence(context->device, textureData->fence, nullptr);
        vkDestroyImage(context->device, textureData->image, nullptr);
        throw std::runtime_error("failed to create image view!");
    }

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

    if (vkCreateSampler(context->device, &samplerInfo, nullptr, &textureData->sampler) != VK_SUCCESS) {
        vkDestroyImageView(context->device, textureData->view, nullptr);
        vkDestroyImage(context->device, textureData->image, nullptr);
        vkFreeMemory(context->device, textureData->memory, nullptr);
        throw std::runtime_error("failed to create texture sampler!");
    }

    textureData->width = width;
    textureData->height = height;
    textureData->device = context->device;

    return textureData;
}

void CSJVulkanHelper::WaitForTextureUpload(CSJSpTexture &info) {
    auto spTexture = std::dynamic_pointer_cast<TextureData>(info);

    if (spTexture->isComplete) {
        return;
    }

    vkWaitForFences(spTexture->device, 1, &spTexture->fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(spTexture->device, spTexture->fence, nullptr);

    vkDestroyBuffer(spTexture->device,  spTexture->stagingBuffer, nullptr);
    vkFreeMemory(spTexture->device, spTexture->stagingMemory, nullptr);

    spTexture->fence = VK_NULL_HANDLE;
    spTexture->isComplete = true;
}

bool CSJVulkanHelper::IsTextureUploadComplete(CSJSpTexture &info) {
    auto spTexture = std::dynamic_pointer_cast<TextureData>(info);

    if (spTexture->isComplete) {
        return true;
    }

    VkResult result = vkGetFenceStatus(spTexture->device, spTexture->fence);
    if (result == VK_SUCCESS) {
        vkDestroyFence(spTexture->device, spTexture->fence, nullptr);
        spTexture->fence = VK_NULL_HANDLE;
        spTexture->isComplete = true;
        return true;
    }

    return false;
}

void CSJVulkanHelper::DestroyTexture(CSJSpTexture &info) {
    auto spTexture = std::dynamic_pointer_cast<TextureData>(info);

    if (!spTexture->isComplete) {
        vkWaitForFences(spTexture->device, 1, &spTexture->fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(spTexture->device, spTexture->fence, nullptr);
    }

    if (spTexture->view != VK_NULL_HANDLE) {
        vkDestroyImageView(spTexture->device, spTexture->view, nullptr);
    }
    if (spTexture->image != VK_NULL_HANDLE) {
        vkDestroyImage(spTexture->device, spTexture->image, nullptr);
    }
}

CSJSpTexture CSJVulkanHelper::CreateTextureFromFile(CSJVulkanHelperContext *context,
                                                         const std::string &filePath) {
    if (filePath.empty()) {
        return nullptr;
    }

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4; // force using rgba.
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    return CreateTexture2D(context,
                           texWidth,
                           texHeight,
                           ToVkFormat(CSJPixelFormat::CSJPixelFormat_R8G8B8A8_SRGB),
                           pixels,
                           imageSize);

}

void CSJVulkanHelper::UpdateTexture(ICSJTexture *texture, const void *data, size_t dataSize) {
    /**  For now, just a placeholder.
     * You can implement partial updates here.
     * This would involve creating a staging buffer and copying.
     */
    throw std::runtime_error("UpdateTexture not yet implemented.");
}

void CSJVulkanHelper::DestroyTexture(VkDevice device, ICSJTexture *texture) {
    if (!texture) {
        return;
    }

    TextureData* texData = static_cast<TextureData*>(texture);

    // Wait for GPU to finish
    vkDeviceWaitIdle(device);

    if (texData->sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, texData->sampler, nullptr);
    }

    if (texData->view != VK_NULL_HANDLE) {
        vkDestroyImageView(device, texData->view, nullptr);
    }

    if (texData->image != VK_NULL_HANDLE) {
        vkDestroyImage(device, texData->image, nullptr);
    }

    if (texData->memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, texData->memory, nullptr);
    }

    delete texData;
}

CSJSpBuffer CSJVulkanHelper::CreateBuffer(VkDevice device,
                                          VkPhysicalDevice physical_device,
                                          size_t size,
                                          CSJBufferUsage usage,
                                          const void *data) {
    VkBufferUsageFlags vkUsage = ToVkBufferUsage(usage);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = vkUsage;

    VkBuffer buffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

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
        vkDestroyBuffer(device, buffer, nullptr);
        throw std::runtime_error("failed to find suitable memory type for buffer!");
    }

    VkDeviceMemory memory;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        vkDestroyBuffer(device, buffer, nullptr);
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, memory, 0);

    // If data is provided, upload it
    if (data && size > 0 && usage == CSJBufferUsage::CSJBufferUsage_Staging) {
        void* mappedData;
        vkMapMemory(device, memory, 0, size, 0, &mappedData);
        memcpy(mappedData, data, size);
        vkUnmapMemory(device, memory);
    }

    auto bufferData = std::make_unique<BufferData>();
    bufferData->buffer = buffer;
    bufferData->memory = memory;
    bufferData->size = size;
    bufferData->usage = usage;

    return bufferData;
}

void CSJVulkanHelper::UpdateBuffer(VkDevice device,
                                   ICSJBuffer *buffer,
                                   const void *data,
                                   size_t dataSize,
                                   size_t offset) {
    if (!buffer || !data) {
        return;
    }

    BufferData* bufData = static_cast<BufferData*>(buffer);

    void* mappedData;
    vkMapMemory(device, bufData->memory, offset, dataSize, 0, &mappedData);
    memcpy(mappedData, data, dataSize);
    vkUnmapMemory(device, bufData->memory);
}

void CSJVulkanHelper::DestroyBuffer(VkDevice device, ICSJBuffer *buffer) {
    if (!buffer) {
        return;
    }

    BufferData* bufData = static_cast<BufferData*>(buffer);

    vkDestroyBuffer(device, bufData->buffer, nullptr);
    vkFreeMemory(device, bufData->memory, nullptr);

    delete bufData;
}

std::string CSJVulkanHelper::GetBackendName() {
    return "Vulkan";
}

VkFormat CSJVulkanHelper::ToVkFormat(CSJPixelFormat format) {
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

VkBufferUsageFlags CSJVulkanHelper::ToVkBufferUsage(CSJBufferUsage usage) {
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

VkCommandBuffer CSJVulkanHelper::BeginSingleTimeCommands(VkDevice device, VkCommandPool commadPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commadPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CSJVulkanHelper::EndSingleTimeCommands(VkDevice device, 
                                            VkCommandBuffer commandBuffer,
                                            VkCommandPool commandPool,
                                            VkQueue queue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void CSJVulkanHelper::CopyBufferToImage(VkDevice device,
                                        VkCommandPool commandPool,
                                        VkQueue queue,
                                        VkBuffer buffer,
                                        VkImage image,
                                        uint32_t width, 
                                        uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

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

    EndSingleTimeCommands(device, commandBuffer, commandPool, queue);
}

}
