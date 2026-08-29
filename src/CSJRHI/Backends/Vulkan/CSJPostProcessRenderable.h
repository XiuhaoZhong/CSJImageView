#pragma once

#include "ICSJRenderable.h"

#include <memory>

#include <vulkan/vulkan.h>

namespace csjrhi {

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

    const char* GatName() const override {
        return "PostProcessRenderable";
    }

    void setInputTexture(VkImageView imageView, VkSampler sampler);

    VkFramebuffer getFramebuffer() const {
        return m_offscreenFramebuffer;
    }

    VkImage getImage() const {
        return m_offscreenImage;
    }

    void recreateOffscreenImage();
protected:
    void createOffscreenImage();
    void createPipeline();

private:
    void    *m_render_handler = nullptr;

    // Offscreen resources
    VkImage         m_offscreenImage        = VK_NULL_HANDLE;
    VkDeviceMemory  m_offscreenMemory       = VK_NULL_HANDLE;
    VkImageView     m_offscreenImageView    = VK_NULL_HANDLE;
    VkFramebuffer   m_offscreenFramebuffer  = VK_NULL_HANDLE;

    VkImageView     m_inputImageView;
    VkSampler       m_sampler;

    VkRenderPass    m_offscreenRenderPass   = VK_NULL_HANDLE;  // Same as scene render pass

    VkDescriptorSetLayout m_descriptorset_layout;
    VkPipelineLayout      m_pipeline_layout;
    // Post-process resources
    VkPipeline            m_postProcessPipeline             = VK_NULL_HANDLE;
    VkPipelineLayout      m_postProcessPipelineLayout       = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_postProcessDescriptorSetLayout  = VK_NULL_HANDLE;
    VkDescriptorSet       m_postProcessDescriptorSet        = VK_NULL_HANDLE;
};

using CSJSpPostProcessRenderable = std::shared_ptr<CSJPostProcessRenderable>;

}