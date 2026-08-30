#pragma once 

#include "ICSJRenderable.h"

#include <vulkan/vulkan.h>

#include "CSJVulkanHelper.h"

namespace csjrhi {

class CSJImageRenderable : public ICSJRenderable {
public:
    CSJImageRenderable();
    ~CSJImageRenderable();

    bool init(void* rendererHanle) override;
    bool isReady() const override;
    void updateScene() override;
    void render(void* commandHandle, float timeStamp) override;
    void onResize(uint32_t width, uint32_t height) override;
    void unInit() override;

    const char* GatName() const override;

protected:
    void createDescriptorSetLayout();
    void createPipeline();
    void createImageTexture();
    void createDescriptorSets();
    void createUniformBuffers();
    void createVertexBuffer();
    void createIndexBuffer();
    void updateUniformBuffer(uint32_t currentImage);

private:
    void                 *m_render_handler = nullptr;
    int                   m_current_frame = 0;

    VkDevice              m_device;
    VkDescriptorSetLayout m_descriptorset_layout;
    VkPipelineLayout      m_pipeline_layout;
    VkPipeline            m_graphics_pipeline;

    VkBuffer              m_vertex_buffer;
    VkDeviceMemory        m_vertex_buffer_memory;
    VkBuffer              m_index_buffer;
    VkDeviceMemory        m_index_buffer_memory;

    std::vector<VkDescriptorSet> m_descriptor_sets;
    std::vector<VkBuffer>        m_uniform_buffers;
    std::vector<VkDeviceMemory>  m_uniform_buffer_memories;
    std::vector<void *>          m_uniform_buffer_mappeds;

    CSJSpTexture m_pTexData = nullptr;
};
}