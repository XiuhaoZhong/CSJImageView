#include "CSJYUVRenderable.h"

#include <iostream>
#include <array>

#include "CSJVulkanRenderer.h"

namespace csjrhi {

CSJYUVRenderable::CSJYUVRenderable() {

}

CSJYUVRenderable::~CSJYUVRenderable() {
}

bool CSJYUVRenderable::init(void *rendererHanle) {
    m_render_handler = rendererHanle;
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    m_device = renderer->getDevice();

    createYUVStorageImages();
    createYUVImageViews();
    createYUVSampler();
    createYUVUniformBuffer();

    // Create compute pipeline
    createYUVComputeDescriptorSetLayout();
    createYUVComputePipelineLayout();
    createYUVComputePipeline();
    createYUVComputeDescriptorSet();

    // Create graphics pipeline
    createYUVDescriptorSetLayout();
    createYUVPipelineLayout();
    createYUVPipeline();
    createYUVDescriptorSet();

    return false;
}

bool CSJYUVRenderable::isReady() const {

    return false;
}

void CSJYUVRenderable::updateScene() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkCommandBuffer commandBuffer = renderer->getCommandBuffer();

    TransitionYUVToGeneral(commandBuffer);
    generateYUVFrame(commandBuffer, m_time);
    TransitionYUVToSampling(commandBuffer);
    m_time += 0.016;
}

void CSJYUVRenderable::render(void *commandHandle, float timeStamp) {
    if (!m_render_handler) {
        return ;
    }

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkExtent2D curExtent = renderer->getSwapchainExtent();
    VkCommandBuffer commandBuffer = renderer->getCommandBuffer();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_yuvPipeline);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_yuvPipelineLayout,
                            0,
                            1,
                            &m_yuvDescriptorSet,
                            0,
                            nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void CSJYUVRenderable::onResize(uint32_t width, uint32_t height) {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    destroyImageData();

    createYUVStorageImages();

    createYUVImageViews();

    updateDescriptorSets();
    updateGraphicDescriptorSets();
}

void CSJYUVRenderable::unInit() {
    std::cout << "CSJImageRenderable::unInit()" << std::endl;
    // Destroy graphics pipeline resources
    if (m_yuvPipeline) {
        vkDestroyPipeline(m_device, m_yuvPipeline, nullptr);
    }

    if (m_yuvPipelineLayout) {
        vkDestroyPipelineLayout(m_device, m_yuvPipelineLayout, nullptr);
    }

    if (m_yuvDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, m_yuvDescriptorSetLayout, nullptr);
    }

    // Destroy compute pipeline resources
    if (m_yuvComputePipeline) {
        vkDestroyPipeline(m_device, m_yuvComputePipeline, nullptr);
    }

    if (m_yuvComputePipelineLayout) {
        vkDestroyPipelineLayout(m_device, m_yuvComputePipelineLayout, nullptr);
    }

    if (m_yuvComputeDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, m_yuvComputeDescriptorSetLayout, nullptr);
    }

    // Destroy YUV resources
    if (m_yuvSampler) {
        vkDestroySampler(m_device, m_yuvSampler, nullptr);
    }

    if (m_yuvUniformBuffer) {
        vkDestroyBuffer(m_device, m_yuvUniformBuffer, nullptr);
    }

    if (m_yuvUniformBufferMemory) {
        vkFreeMemory(m_device, m_yuvUniformBufferMemory, nullptr);
    }

    destroyImageData();
}

void CSJYUVRenderable::createYUVStorageImages() {

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);
    VkExtent2D curExtent = renderer->getSwapchainExtent();

    auto createImage = [&](uint32_t w, uint32_t h, VkFormat fmt, VkImage& img, VkDeviceMemory& mem) {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent = {w, h, 1};
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.format = fmt;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.samples = VK_SAMPLE_COUNT_1_BIT;

        vkCreateImage(m_device, &info, nullptr, &img);

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(m_device, img, &req);

        VkMemoryAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = req.size;

        // Find device-local memory type
        VkPhysicalDeviceMemoryProperties props;
        vkGetPhysicalDeviceMemoryProperties(renderer->getPhysicalDevice(), &props);
        for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
            if (req.memoryTypeBits & (1 << i)) {
                if (props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                    alloc.memoryTypeIndex = i;
                    break;
                }
            }
        }

        vkAllocateMemory(m_device, &alloc, nullptr, &mem);
        vkBindImageMemory(m_device, img, mem, 0);
    };

    auto extent = curExtent;

    std::cout << "create yuv image, size = " << extent.width << "x" << extent.height << std::endl;

    createImage(extent.width, extent.height, VK_FORMAT_R8_UNORM, m_yuvYImage, m_yuvYMemory);
    createImage(extent.width / 2, extent.height / 2, VK_FORMAT_R8_UNORM, m_yuvUImage, m_yuvUMemory);
    createImage(extent.width / 2, extent.height / 2, VK_FORMAT_R8_UNORM, m_yuvVImage, m_yuvVMemory);
}

void CSJYUVRenderable::createYUVImageViews() {
    auto createView = [&](VkImage img, VkFormat fmt, VkImageView& view) {
        VkImageViewCreateInfo info{};
        info.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image                       = img;
        info.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
        info.format                      = fmt;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        vkCreateImageView(m_device, &info, nullptr, &view);
    };

    createView(m_yuvYImage, VK_FORMAT_R8_UNORM, m_yuvYImageView);
    createView(m_yuvUImage, VK_FORMAT_R8_UNORM, m_yuvUImageView);
    createView(m_yuvVImage, VK_FORMAT_R8_UNORM, m_yuvVImageView);
}

void CSJYUVRenderable::createYUVDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 3> bindings{};

    // Binding 0: Y texture
    bindings[0].binding         = 0;
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Binding 1: U texture
    bindings[1].binding         = 1;
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Binding 2: V texture
    bindings[2].binding         = 2;
    bindings[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_yuvDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create YUV graphics descriptor set layout!");
    }
}

void CSJYUVRenderable::createYUVPipelineLayout() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts    = &m_yuvDescriptorSetLayout;

    if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_yuvPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create YUV graphics pipeline layout!");
    }
}

void CSJYUVRenderable::createYUVPipeline() {

    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    // 1. Load shaders
    auto vertShaderCode = renderer->readFile("resources/shaders/yuv_vert.spv");
    auto fragShaderCode = renderer->readFile("resources/shaders/yuv_frag.spv");

    VkShaderModule vertModule = renderer->createShaderModule(vertShaderCode);
    VkShaderModule fragModule = renderer->createShaderModule(fragShaderCode);

    // 2. Shader stages
    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

    // 3. Vertex input (no vertex buffers)
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount   = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;

    // 4. Input assembly (triangle list)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    int windowWidth =  renderer->getWindowWidth();
    int windowHeight = renderer->getWindowHeight();

    // 5. Viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(windowWidth);
    viewport.height = static_cast<float>(windowHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight)};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    // 6. Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode    = VK_CULL_MODE_NONE;
    rasterizer.frontFace   = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth   = 1.0f;

    // 7. Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable  = VK_FALSE;

    // 8. Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &colorBlendAttachment;

    // 9. Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = stages;
    pipelineInfo.pVertexInputState   = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.layout              = m_yuvPipelineLayout;
    pipelineInfo.renderPass          = renderer->getPostprocessRenderPass();
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_yuvPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create YUV graphics pipeline!");
    }

    // 10. Cleanup shader modules
    vkDestroyShaderModule(m_device, vertModule, nullptr);
    vkDestroyShaderModule(m_device, fragModule, nullptr);
}

void CSJYUVRenderable::createYUVDescriptorSet() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = renderer->getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &m_yuvDescriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_yuvDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate YUV graphics descriptor set!");
    }

    // Update descriptor set
    updateGraphicDescriptorSets();
}

void CSJYUVRenderable::createYUVSampler() {
    VkSamplerCreateInfo info{};
    info.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter    = VK_FILTER_LINEAR;
    info.minFilter    = VK_FILTER_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(m_device, &info, nullptr, &m_yuvSampler);
}

void CSJYUVRenderable::createYUVUniformBuffer() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size  = sizeof(YUVUniforms);
    info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    vkCreateBuffer(m_device, &info, nullptr, &m_yuvUniformBuffer);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(m_device, m_yuvUniformBuffer, &req);

    VkMemoryAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = req.size;

    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(renderer->getPhysicalDevice(), &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if (req.memoryTypeBits & (1 << i)) {
            if (props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                alloc.memoryTypeIndex = i;
                break;
            }
        }
    }

    vkAllocateMemory(m_device, &alloc, nullptr, &m_yuvUniformBufferMemory);
    vkBindBufferMemory(m_device, m_yuvUniformBuffer, m_yuvUniformBufferMemory, 0);
    vkMapMemory(m_device, m_yuvUniformBufferMemory, 0, sizeof(YUVUniforms), 0, &m_yuvUniformBufferMapped);
}

void CSJYUVRenderable::createYUVComputeDescriptorSetLayout() {
    // 3 storage images + 1 uniform buffer
    std::array<VkDescriptorSetLayoutBinding, 4> bindings{};

    // Binding 0: Y storage image
    bindings[0].binding            = 0;
    bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[0].descriptorCount    = 1;
    bindings[0].stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    // Binding 1: U storage image
    bindings[1].binding         = 1;
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 2: V storage image
    bindings[2].binding         = 2;
    bindings[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 3: Uniform buffer
    bindings[3].binding         = 3;
    bindings[3].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_yuvComputeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }
}

void CSJYUVRenderable::createYUVComputePipelineLayout() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount         = 1;
    layoutInfo.pSetLayouts            = &m_yuvComputeDescriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges    = nullptr;

    if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_yuvComputePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }
}

void CSJYUVRenderable::createYUVComputePipeline() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    // 1. Load shader module
    auto shaderCode = renderer->readFile("resources/shaders/yuv_generator.spv");
    VkShaderModule shaderModule = renderer->createShaderModule(shaderCode);

    // 2. Shader stage info
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = shaderModule;
    stageInfo.pName = "main";

    // 3. Create pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = m_yuvComputePipelineLayout;

    if (vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_yuvComputePipeline) != VK_SUCCESS) {
        vkDestroyShaderModule(m_device, shaderModule, nullptr);
        throw std::runtime_error("failed to create compute pipeline!");
    }

    // 4. Cleanup shader module (no longer needed)
    vkDestroyShaderModule(m_device, shaderModule, nullptr);
}

void CSJYUVRenderable::createYUVComputeDescriptorSet() {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    // 1. Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = renderer->getDescriptorPool();// m_descripotrPoolForRenderables;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &m_yuvComputeDescriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_yuvComputeDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute descriptor set!");
    }

    updateDescriptorSets();
}

void CSJYUVRenderable::updateDescriptorSets() {
    // Update descriptor set with storage images and uniform buffer
    std::array<VkDescriptorImageInfo, 3> imageInfos{};
    imageInfos[0].imageView   = m_yuvYImageView;  // Create image views for storage images
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;  // Storage images use GENERAL layout
    imageInfos[1].imageView   = m_yuvUImageView;
    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfos[2].imageView   = m_yuvVImageView;
    imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_yuvUniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(YUVUniforms);

    std::array<VkWriteDescriptorSet, 4> writes{};
    writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet          = m_yuvComputeDescriptorSet;
    writes[0].dstBinding      = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[0].pImageInfo      = &imageInfos[0];

    writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet          = m_yuvComputeDescriptorSet;
    writes[1].dstBinding      = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[1].pImageInfo      = &imageInfos[1];

    writes[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet          = m_yuvComputeDescriptorSet;
    writes[2].dstBinding      = 2;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[2].pImageInfo      = &imageInfos[2];

    writes[3].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[3].dstSet          = m_yuvComputeDescriptorSet;
    writes[3].dstBinding      = 3;
    writes[3].descriptorCount = 1;
    writes[3].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[3].pBufferInfo     = &bufferInfo;

    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void CSJYUVRenderable::updateGraphicDescriptorSets() {
    std::array<VkDescriptorImageInfo, 3> imageInfos{};
    imageInfos[0].sampler     = m_yuvSampler;  // Create a sampler for YUV textures
    imageInfos[0].imageView   = m_yuvYImageView;
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageInfos[1].sampler     = m_yuvSampler;
    imageInfos[1].imageView   = m_yuvUImageView;
    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageInfos[2].sampler     = m_yuvSampler;
    imageInfos[2].imageView   = m_yuvVImageView;
    imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::array<VkWriteDescriptorSet, 3> writes{};
    writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet          = m_yuvDescriptorSet;
    writes[0].dstBinding      = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].pImageInfo      = &imageInfos[0];

    writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet          = m_yuvDescriptorSet;
    writes[1].dstBinding      = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo      = &imageInfos[1];

    writes[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet          = m_yuvDescriptorSet;
    writes[2].dstBinding      = 2;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].pImageInfo      = &imageInfos[2];

    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void CSJYUVRenderable::TransitionYUVToSampling(VkCommandBuffer commandBuffer)
{
    auto barrier = [&](VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        } else {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
        }

        return barrier;
    };

    std::array<VkImageMemoryBarrier, 3> barriers{
        barrier(m_yuvYImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        barrier(m_yuvUImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        barrier(m_yuvVImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    };

    // Source stage: compute shader (the compute shader wrote to the images)
    // Destination stage: fragment shader (the fragment shader will read from them)
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         srcStage,
                         dstStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         static_cast<uint32_t>(barriers.size()),
                         barriers.data());
}

void CSJYUVRenderable::TransitionYUVToGeneral(VkCommandBuffer commandBuffer) {
    std::array<VkImageMemoryBarrier, 3> barriers{};

    std::vector<VkImage> yuvImages = {m_yuvYImage, m_yuvUImage, m_yuvVImage};

    for (int i = 0; i < 3; i++) {
        barriers[i].sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers[i].oldLayout                   = m_textureShowed ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
        barriers[i].newLayout                   = VK_IMAGE_LAYOUT_GENERAL;
        barriers[i].srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
        barriers[i].dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
        barriers[i].image                       = yuvImages[i];
        barriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barriers[i].subresourceRange.levelCount = 1;
        barriers[i].subresourceRange.layerCount = 1;
        barriers[i].srcAccessMask               = VK_ACCESS_SHADER_READ_BIT;
        barriers[i].dstAccessMask               = VK_ACCESS_SHADER_WRITE_BIT;
    }

    m_textureShowed = true;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,   // Wait for previous reads to finish
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,    // Before compute shader writes
        0, 0, nullptr, 0, nullptr,
        static_cast<uint32_t>(barriers.size()),
        barriers.data()
    );
}

void CSJYUVRenderable::generateYUVFrame(VkCommandBuffer commandBuffer, float time) {
    auto *renderer = static_cast<CSJVulkanRenderer *>(m_render_handler);

    auto extent2D = renderer->getSwapchainExtent();
    int windowWidth = extent2D.width;
    int windowHeight = extent2D.height;

    // Update uniform buffer (time, etc.)
    YUVUniforms uniforms{};
    uniforms.time        = time;
    uniforms.aspectRatio = (float)windowWidth / (float)windowHeight;
    memcpy(m_yuvUniformBufferMapped, &uniforms, sizeof(YUVUniforms));

    // Bind compute pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_yuvComputePipeline);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_yuvComputePipelineLayout,
                            0,
                            1,
                            &m_yuvComputeDescriptorSet,
                            0,
                            nullptr);

    // Dispatch
    uint32_t groupX = (extent2D.width + 15) / 16;
    uint32_t groupY = (extent2D.height + 15) / 16;

    vkCmdDispatch(commandBuffer, groupX, groupY, 1);
}

void CSJYUVRenderable::destroyImageData() {
    if (m_yuvYImage) {
        vkDestroyImage(m_device, m_yuvYImage, nullptr);
    }

    if (m_yuvUImage) {
        vkDestroyImage(m_device, m_yuvUImage, nullptr);
    }

    if (m_yuvVImage) {
        vkDestroyImage(m_device, m_yuvVImage, nullptr);
    }

    if (m_yuvYMemory) {
        vkFreeMemory(m_device, m_yuvYMemory, nullptr);
    }

    if (m_yuvUMemory) {
        vkFreeMemory(m_device, m_yuvUMemory, nullptr);
    }

    if (m_yuvVMemory) {
        vkFreeMemory(m_device, m_yuvVMemory, nullptr);
    }

    if (m_yuvYImageView) {
        vkDestroyImageView(m_device, m_yuvYImageView, nullptr);
    }

    if (m_yuvUImageView) {
        vkDestroyImageView(m_device, m_yuvUImageView, nullptr);
    }

    if (m_yuvVImageView) {
        vkDestroyImageView(m_device, m_yuvVImageView, nullptr);
    }

    m_textureShowed = false;
}

} // namespace csjrhi;
