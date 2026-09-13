#pragma once

#include "ICSJRenderable.h"

#include <memory>
#include <mutex>
#include <atomic>
#include <array>

#include <vulkan/vulkan.h>

namespace csjrhi {

constexpr uint32_t post_process_max_frames = 2;

class CSJPostProcessRenderable : public ICSJRenderable {
public:
    CSJPostProcessRenderable();
    ~CSJPostProcessRenderable();

    bool init(void* rendererHanle) override;
    bool isReady() const override;
    void updateScene() override;
    void render(void* commandHandle, float timeStamp) override;
    void onResize(uint32_t width, uint32_t height) override;
    void unInit() override;

    void setEffectParam(CSJEffectParams effectParam);

    void transitionOffscreenToColorAttachment(VkCommandBuffer commandBuffer);
    void transitionOffscreenToShaderReadOnly(VkCommandBuffer commandBuffer);

    const char* GatName() const override {
        return "PostProcessRenderable";
    }

    void setInputTexture(VkImageView imageView, VkSampler sampler);

    VkFramebuffer getOffscreenFramebuffer() const {
        return m_offscreenFramebuffer;
    }

    VkRenderPass getOffscreenRenderPass() const {
        return m_offscreenRenderPass;
    }

    VkImageView getOffscreenImageView() const {
        return m_offscreenImageView;
    }

    VkImage getImage() const {
        return m_offscreenImage;
    }

    VkRenderPass getRenderPass() const {
        return m_offscreenRenderPass;
    }

    void recreateOffscreenImage();
protected:
    struct EffectUniform {
        VkBuffer       buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void*          buffer_mapped = nullptr;
    };

    void createOffscreenResources();
    void destroyOffscreenResource();
    void createOffscreenFramebuffer();

    void createEffectUniformBuffers();
    void destroyEffectUniformBuffers();

    void createRenderPass();
    void createSampler();
    void createDescriptorSetLayout();
    void createDescriptorSet();
    void updateDescriptorSet();

    void createPipeline();
    void reCreatePipeline();

private:
    void    *m_render_handler = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_current_index = 0;

    std::array<EffectUniform, post_process_max_frames> m_effectBuffers;

    std::mutex        m_effectMtx;
    CSJEffectParams   m_effectParam;

    VkFormat m_format = VK_FORMAT_UNDEFINED;
    // Offscreen resources
    VkImage         m_offscreenImage        = VK_NULL_HANDLE;
    VkDeviceMemory  m_offscreenMemory       = VK_NULL_HANDLE;
    VkImageView     m_offscreenImageView    = VK_NULL_HANDLE;
    VkFramebuffer   m_offscreenFramebuffer  = VK_NULL_HANDLE;
    VkRenderPass    m_offscreenRenderPass   = VK_NULL_HANDLE;
    VkImageLayout   m_offscreenLayout       = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageView     m_inputImageView        = VK_NULL_HANDLE;
    VkSampler       m_sampler               = VK_NULL_HANDLE;

    // Post-process resources
    VkDescriptorSetLayout m_descriptorset_layout            = VK_NULL_HANDLE;
    VkPipeline            m_postProcessPipeline             = VK_NULL_HANDLE;
    VkPipelineLayout      m_postProcessPipelineLayout       = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_postProcessDescriptorSetLayout  = VK_NULL_HANDLE;
    VkDescriptorSet       m_postProcessDescriptorSet        = VK_NULL_HANDLE;
    std::array<VkDescriptorSet, post_process_max_frames> m_descriptorSet;

    /* Just keep, not allocate and deallocate. */
    VkDevice         m_device          = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkQueue          m_graphics_queue  = VK_NULL_HANDLE;
    VkCommandPool    m_command_pool    = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
};

using CSJSpPostProcessRenderable = std::shared_ptr<CSJPostProcessRenderable>;

}