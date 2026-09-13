#include "CSJPostProcessRenderable.h"

#include <iostream>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CSJVulkanRenderer.h"

namespace csjrhi {

CSJPostProcessRenderable::CSJPostProcessRenderable() {

}

CSJPostProcessRenderable::~CSJPostProcessRenderable() {

}

bool CSJPostProcessRenderable::init(void *rendererHanle) {
    m_render_handler = rendererHanle;

    auto *renderer      = static_cast<CSJVulkanRenderer *>(m_render_handler);
    m_width             = renderer->getSwapchainExtent().width;
    m_height            = renderer->getSwapchainExtent().height;
    m_device            = renderer->getDevice();
    m_physical_device   = renderer->getPhysicalDevice();
    m_graphics_queue    = renderer->getGraphicsQueue();
    m_command_pool      = renderer->getCommandPool();
    m_descriptor_pool   = renderer->getDescriptorPool();

    createOffscreenResources();
    createRenderPass();
    createOffscreenFramebuffer();
    createSampler();
    createEffectUniformBuffers();
    createDescriptorSetLayout();
    createPipeline();
    createDescriptorSet();

    setInputTexture(m_offscreenImageView, m_sampler);

    m_effectParam.effectType = 0;
    m_effectParam.exposure = 1.0;
    m_effectParam.intensity = 1.0;

    return true;
}

bool CSJPostProcessRenderable::isReady() const {
    return false;
}

void CSJPostProcessRenderable::updateScene() {
    std::lock_guard<std::mutex> guard_lock(m_effectMtx);
    memcpy(m_effectBuffers[m_current_index].buffer_mapped,
           &m_effectParam,
           sizeof(CSJEffectParams));
}

void CSJPostProcessRenderable::render(void *commandHandle, float timeStamp) {
    updateDescriptorSet();

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkCommandBuffer commandBuffer = renderer->getCommandBuffer();

    // 1. Bind the post-process pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postProcessPipeline);

    // 2. Bind the descriptor set (contains the offscreen texture)
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_postProcessPipelineLayout,
                            0,                                 // first set index
                            1,                                 // set count
                            &m_descriptorSet[m_current_index], // the descriptor
                            0,                                 // dynamic offset count
                            nullptr);                          // dynamic offsets

    // 3. Draw a full-screen quad (6 vertices = 2 triangles)
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    m_current_index = (m_current_index + 1) % post_process_max_frames;
}

void CSJPostProcessRenderable::onResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    vkDeviceWaitIdle(m_device);

    destroyOffscreenResource();
    createOffscreenResources();

    if (m_offscreenFramebuffer) {
        vkDestroyFramebuffer(m_device, m_offscreenFramebuffer, nullptr);
        m_offscreenFramebuffer = VK_NULL_HANDLE;
    }
    createOffscreenFramebuffer();

    setInputTexture(m_offscreenImageView, m_sampler);

    m_offscreenLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void CSJPostProcessRenderable::unInit() {
    std::cout << "CSJPostProcessRenderable::unInit()" << std::endl;

    if (m_postProcessPipeline) {
        vkDestroyPipeline(m_device, m_postProcessPipeline, nullptr);
    }

    if (m_postProcessPipelineLayout) {
        vkDestroyPipelineLayout(m_device, m_postProcessPipelineLayout, nullptr);
    }

    if (m_postProcessDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, m_postProcessDescriptorSetLayout, nullptr);
    }

    if (m_offscreenFramebuffer) {
        vkDestroyFramebuffer(m_device, m_offscreenFramebuffer, nullptr);
    }

    if (m_offscreenImageView) {
        vkDestroyImageView(m_device, m_offscreenImageView, nullptr);
    }

    if (m_offscreenMemory) {
        vkFreeMemory(m_device, m_offscreenMemory, nullptr);
    }

    if (m_offscreenImage) {
        vkDestroyImage(m_device, m_offscreenImage, nullptr);
    }

    if (m_sampler) {
        vkDestroySampler(m_device, m_sampler, nullptr);
    }

    if (m_offscreenRenderPass) {
        vkDestroyRenderPass(m_device, m_offscreenRenderPass, nullptr);
    }

    destroyEffectUniformBuffers();
}

void CSJPostProcessRenderable::setEffectParam(CSJEffectParams effectParam) {
    std::lock_guard<std::mutex> guard_lock(m_effectMtx);
    // m_effectParam = effectParam;

    m_effectParam = effectParam;

    std::cout << "m_effectParam: " << "(" << m_effectParam.effectType 
                                   << ", " << m_effectParam.exposure 
                                   << ", " << m_effectParam.intensity
                                   << ")" << std::endl;
}

void CSJPostProcessRenderable::setInputTexture(VkImageView imageView, VkSampler sampler) {
    m_inputImageView = imageView;
    m_sampler = sampler;
}

void CSJPostProcessRenderable::createPipeline() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    VkSurfaceFormatKHR surfaceFormat = renderer->getSurfaceFormat();
    
    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &m_postProcessDescriptorSetLayout;

    vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_postProcessPipelineLayout);

    auto vertShaderCode = renderer->readFile("resources/shaders/post_process_vert.spv");
    auto fragShaderCode = renderer->readFile("resources/shaders/post_process_frag.spv");

    VkShaderModule vertShaderModule = renderer->createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = renderer->createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_NONE;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable  = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                            VK_COLOR_COMPONENT_G_BIT | 
                                            VK_COLOR_COMPONENT_B_BIT | 
                                            VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = m_postProcessPipelineLayout;
    pipelineInfo.renderPass          = renderer->getRenderPass();
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, 
                                  &pipelineInfo, nullptr, 
                                  &m_postProcessPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void CSJPostProcessRenderable::reCreatePipeline() {
    if (m_postProcessPipeline) {
        vkDestroyPipeline(m_device, m_postProcessPipeline, nullptr);
    }

    createPipeline();
}

void CSJPostProcessRenderable::transitionOffscreenToColorAttachment(VkCommandBuffer commandBuffer) {
    VkImageMemoryBarrier barrier{};
    barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                   = m_offscreenLayout;
    barrier.newLayout                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_offscreenImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask               = 0;
    barrier.dstAccessMask               = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    m_offscreenLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void CSJPostProcessRenderable::transitionOffscreenToShaderReadOnly(VkCommandBuffer commandBuffer){
    VkImageMemoryBarrier barrier{};
    barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                   = m_offscreenLayout;
    barrier.newLayout                   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                       = m_offscreenImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask               = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask               = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    m_offscreenLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void CSJPostProcessRenderable::createOffscreenResources() {
    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = VK_FORMAT_R16G16B16A16_SFLOAT;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &m_offscreenImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen image!");
    }

    // 2. Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, m_offscreenImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    // Find device-local memory type
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (memRequirements.memoryTypeBits & (1 << i)) {
            if (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                allocInfo.memoryTypeIndex = i;
                break;
            }
        }
    }

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_offscreenMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate offscreen image memory!");
    }

    vkBindImageMemory(m_device, m_offscreenImage, m_offscreenMemory, 0);

    // 3. Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                       = m_offscreenImage;
    viewInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                      = VK_FORMAT_R16G16B16A16_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_offscreenImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen image view!");
    }
}

void CSJPostProcessRenderable::destroyOffscreenResource() {
    if (m_offscreenFramebuffer) {
        vkDestroyFramebuffer(m_device, m_offscreenFramebuffer, nullptr);
        m_offscreenFramebuffer = VK_NULL_HANDLE;
    }
    if (m_offscreenImageView) {
        vkDestroyImageView(m_device, m_offscreenImageView, nullptr);
        m_offscreenImageView = VK_NULL_HANDLE;
    }
    if (m_offscreenImage) {
        vkDestroyImage(m_device, m_offscreenImage, nullptr);
        m_offscreenImage = VK_NULL_HANDLE;
    }
    if (m_offscreenMemory) {
        vkFreeMemory(m_device, m_offscreenMemory, nullptr);
        m_offscreenMemory = VK_NULL_HANDLE;
    }
}

void CSJPostProcessRenderable::createOffscreenFramebuffer() {
    VkImageView attachments[] = {m_offscreenImageView};

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass      = m_offscreenRenderPass;
    fbInfo.attachmentCount = 1;
    fbInfo.pAttachments    = attachments;
    fbInfo.width           = m_width;
    fbInfo.height          = m_height;
    fbInfo.layers          = 1;

    if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &m_offscreenFramebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen framebuffer!");
    }
}

void CSJPostProcessRenderable::createEffectUniformBuffers() {
    VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint64_t buffer_size = static_cast<uint64_t>(sizeof(EffectUniform));

    for (auto& effect_buffer : m_effectBuffers) {
        bool res = CSJVulkanHelper::createUniformBuffer(m_device,
                                                        m_physical_device,
                                                        buffer_size,
                                                        requiredFlags,
                                                        effect_buffer.buffer,
                                                        effect_buffer.memory,
                                                        &effect_buffer.buffer_mapped);
        if (!res) {
            std::runtime_error("Create effect uniform buffer failed!");
        }
    }
}

void CSJPostProcessRenderable::destroyEffectUniformBuffers() {
    for (auto& effect_buffer : m_effectBuffers) {
        if (effect_buffer.buffer) {
            vkDestroyBuffer(m_device, effect_buffer.buffer, nullptr);
        }

        if (effect_buffer.memory) {
            vkFreeMemory(m_device, effect_buffer.memory, nullptr);
            effect_buffer.buffer_mapped = nullptr;
        }
    }
}

void CSJPostProcessRenderable::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format        = VK_FORMAT_R16G16B16A16_SFLOAT;//m_format;
    colorAttachment.samples       = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_offscreenRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen render pass!");
    }
}

void CSJPostProcessRenderable::createSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create sampler!");
    }
}

void CSJPostProcessRenderable::createDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding,2> bindings{};
    bindings[0].binding         = 0;
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Uniform buffer
    bindings[1].binding         = 1;
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_postProcessDescriptorSetLayout);
}

void CSJPostProcessRenderable::createDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_descriptor_pool;
    allocInfo.descriptorSetCount = 2;
    allocInfo.pSetLayouts        = &m_postProcessDescriptorSetLayout;

    for (auto &descriptorSet : m_descriptorSet) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = m_descriptor_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &m_postProcessDescriptorSetLayout;

        vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet);
    }
}

void CSJPostProcessRenderable::updateDescriptorSet() {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView   = m_inputImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler     = m_sampler;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_effectBuffers[m_current_index].buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(EffectUniform);

    std::array<VkWriteDescriptorSet, 2> writes{};
    writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet          = m_descriptorSet[m_current_index];
    writes[0].dstBinding      = 0;
    writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].descriptorCount = 1;
    writes[0].pImageInfo      = &imageInfo;

    writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet          = m_descriptorSet[m_current_index];
    writes[1].dstBinding      = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[1].pBufferInfo     = &bufferInfo;

    vkUpdateDescriptorSets(m_device, 2, writes.data(), 0, nullptr);
}

} // namespace csjrhi;
