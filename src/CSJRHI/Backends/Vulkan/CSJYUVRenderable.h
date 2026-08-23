#pragma once

#include "ICSJRenderable.h"

#include <vulkan/vulkan.h>

namespace csjrhi {

class CSJYUVRenderable : public ICSJRenderable {
public:
    CSJYUVRenderable();
    ~CSJYUVRenderable();

     bool init(void* rendererHanle) override;
     bool isReady() const override;
     void updateScene() override;
     void render(void* commandHandle, float timeStamp) override;
     void onResize(uint32_t width, uint32_t height) override;
     void unInit() override;

     const char* GatName() const override {
        return "";
     }

protected:
    void createYUVStorageImages();
    void createYUVImageViews();
    void createYUVDescriptorSetLayout();
    void createYUVPipelineLayout();
    void createYUVPipeline();
    void createYUVDescriptorSet();
    void createYUVSampler();
    void createYUVUniformBuffer();

    void createYUVComputeDescriptorSetLayout();
    void createYUVComputePipelineLayout();
    void createYUVComputePipeline();
    void createYUVComputeDescriptorSet();

    void updateDescriptorSets();
    void updateGraphicDescriptorSets();
    void TransitionYUVToSampling(VkCommandBuffer commandBuffer);
    void TransitionYUVToGeneral(VkCommandBuffer commandBuffer);
    void generateYUVFrame(VkCommandBuffer commandBuffer, float time);

    void destroyImageData();

private:
    void                 *m_render_handler = nullptr;
    int                   m_current_frame = 0;
    float m_time = 0.0f;
    bool m_textureShowed = false;

    VkDevice              m_device;
    VkDescriptorSetLayout m_yuvDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout      m_yuvPipelineLayout      = VK_NULL_HANDLE;
    VkPipeline            m_yuvPipeline            = VK_NULL_HANDLE;
    VkDescriptorSet       m_yuvDescriptorSet       = VK_NULL_HANDLE;
    VkSampler             m_yuvSampler             = VK_NULL_HANDLE;

    VkImage     m_yuvYImage     = VK_NULL_HANDLE;
    VkImageView m_yuvYImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_yuvYMemory = VK_NULL_HANDLE;
    VkImage     m_yuvUImage     = VK_NULL_HANDLE;
    VkImageView m_yuvUImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_yuvUMemory = VK_NULL_HANDLE;
    VkImage     m_yuvVImage     = VK_NULL_HANDLE;
    VkImageView m_yuvVImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_yuvVMemory = VK_NULL_HANDLE;

    VkBuffer m_yuvUniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_yuvUniformBufferMemory = VK_NULL_HANDLE;
    void* m_yuvUniformBufferMapped = nullptr;

    VkDescriptorSetLayout m_yuvComputeDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout      m_yuvComputePipelineLayout      = VK_NULL_HANDLE;
    VkPipeline            m_yuvComputePipeline            = VK_NULL_HANDLE;
    VkDescriptorSet       m_yuvComputeDescriptorSet       = VK_NULL_HANDLE;
};

} // namespace csjrhi;