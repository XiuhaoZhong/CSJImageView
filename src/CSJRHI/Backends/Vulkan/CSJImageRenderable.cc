#include "CSJImageRenderable.h"

#include <iostream>
#include <vector>
#include <array>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CSJVulkanRenderer.h"

namespace csjrhi {

static int max_frame_in_flight = 2;

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

const std::vector<Vertex> img_vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
};

const std::vector<uint16_t> indices = {
    0,1,2,2,3,0
};

CSJImageRenderable::CSJImageRenderable() {

}

CSJImageRenderable::~CSJImageRenderable() {

}

bool CSJImageRenderable::init(void *rendererHanle) {

    m_render_handler = rendererHanle;

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    m_device = renderer->getDevice();

    createDescriptorSetLayout();
    
    createPipeline();
    createUniformBuffers();
    createImageTexture();
    createDescriptorSets();
    createVertexBuffer();
    createIndexBuffer();
   
    
    return true;
}

bool CSJImageRenderable::isReady() const {
    return false;
}

void CSJImageRenderable::updateScene() {
    
}

void CSJImageRenderable::render(void *commandHandle, float timeStamp) {
    if (!m_render_handler) {
        return ;
    }

    updateUniformBuffer(m_current_frame);

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkExtent2D curExtent = renderer->getSwapchainExtent();

    VkCommandBuffer commandBuffer = renderer->getCommandBuffer();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float) curExtent.width;
    viewport.height   = (float) curExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = curExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);    

    VkBuffer vertexBuffers[] = {
        m_vertex_buffer
    };        
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout, 0, 1, &m_descriptor_sets[m_current_frame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0,  0);

    m_current_frame = (m_current_frame + 1) % max_frame_in_flight;
}

void CSJImageRenderable::onResize(uint32_t width, uint32_t height) {

}

void CSJImageRenderable::unInit() {

    vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);

    vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);

    vkDestroyBuffer(m_device, m_index_buffer, nullptr);
    vkFreeMemory(m_device, m_index_buffer_memory, nullptr);

    for (size_t i = 0 ; i < max_frame_in_flight; i++) {
        vkDestroyBuffer(m_device, m_uniform_buffers[i], nullptr);
        vkFreeMemory(m_device, m_uniform_buffer_memories[i], nullptr);
    }

    m_pTexData.reset();
}

const char *CSJImageRenderable::GatName() const {
    return nullptr;
}

void CSJImageRenderable::createDescriptorSetLayout() {
    if (!m_render_handler) {
        return ;
    }

    auto* renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding            = 0;
    uboLayoutBinding.descriptorCount    = 1;
    uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding            = 1;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorset_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

void CSJImageRenderable::createPipeline() {
    if (!m_render_handler) {
        return ;
    }

    auto* renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    auto vertShaderCode = renderer->readFile("resources/shaders/vert.spv");
    auto fragShaderCode = renderer->readFile("resources/shaders/frag.spv");

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

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

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
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &m_descriptorset_layout;
    //pipelineLayoutInfo.pushConstantRangeCount = 0;
    
    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, 
                               nullptr, &m_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

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
    pipelineInfo.layout              = m_pipeline_layout;
    pipelineInfo.renderPass          = renderer->getRenderPass();
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, 
                                  &pipelineInfo, nullptr, 
                                  &m_graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void CSJImageRenderable::createImageTexture() {
    auto* renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    std::string imagePath("E:\\technology\\Personal_Projects\\CSJImageView\\resources\\originImages\\slamDumk_images\\cross_street.jpeg");

    m_pTexData = renderer->getHelper()->CreateTextureFromFile(imagePath);
}

void CSJImageRenderable::createDescriptorSets() {
    auto* renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    std::vector<VkDescriptorSetLayout> layouts(max_frame_in_flight, m_descriptorset_layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = renderer->getDescriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(max_frame_in_flight);
    allocInfo.pSetLayouts        = layouts.data();
    
    m_descriptor_sets.resize(max_frame_in_flight);
    if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    m_uniform_buffers.resize(max_frame_in_flight);
    for (size_t i = 0; i < max_frame_in_flight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniform_buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = static_cast<VkImageView>(m_pTexData->GetNativeHandle());
        imageInfo.sampler     = static_cast<VkSampler>(m_pTexData->GetSampler());

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet          = m_descriptor_sets[i];
        descriptorWrites[0].dstBinding      = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet          = m_descriptor_sets[i];
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(m_device, 
                               static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(),
                               0, nullptr);
    }
}

void CSJImageRenderable::createUniformBuffers() {
    if (!m_render_handler) {
        return ;
    }

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniform_buffers.resize(max_frame_in_flight);
    m_uniform_buffer_memories.resize(max_frame_in_flight);
    m_uniform_buffer_mappeds.resize(max_frame_in_flight);

    for (size_t i = 0 ; i < max_frame_in_flight; i++) {
        renderer->createBuffer(bufferSize,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               m_uniform_buffers[i],
                               m_uniform_buffer_memories[i]);
        
        vkMapMemory(m_device, m_uniform_buffer_memories[i], 0, bufferSize, 0, &m_uniform_buffer_mappeds[i]);
    }
}

void CSJImageRenderable::createVertexBuffer() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    std::vector<Vertex> current_vertices = img_vertices;
    VkDeviceSize bufferSize = sizeof(current_vertices[0]) * current_vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    renderer->createBuffer(bufferSize, 
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           stagingBuffer,
                           stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, current_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    renderer->createBuffer(bufferSize, 
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           m_vertex_buffer,
                           m_vertex_buffer_memory);
    
    renderer->copyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void CSJImageRenderable::createIndexBuffer() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    renderer->createBuffer(bufferSize, 
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    renderer->createBuffer(bufferSize, 
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           m_index_buffer, m_index_buffer_memory);

    renderer->copyBuffer(stagingBuffer, m_index_buffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void CSJImageRenderable::updateUniformBuffer(uint32_t currentImage) {
    if (!m_render_handler) {
        return ;
    }

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkExtent2D extent = renderer->getSwapchainExtent();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model    = glm::rotate(glm::mat4(1.0f), 0.0f/*time * glm::radians(90.0f)*/, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view     = glm::mat4(1.0f);//glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    float aspect = (float) extent.width / (float) extent.height;
    ubo.proj     = glm::mat4(1.0f);//glm::perspective(glm::radians(1.0f), aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_uniform_buffer_mappeds[m_current_frame], &ubo, sizeof(ubo));
}

}