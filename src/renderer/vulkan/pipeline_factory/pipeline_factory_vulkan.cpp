#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include <array>

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/ubo.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<ResourceManager>& resourceManager, const sp<VKRenderer>& renderFactory)
    : _resource_manager(resourceManager), _renderer(renderFactory)
{
}

PipelineFactoryVulkan::~PipelineFactoryVulkan()
{
}

sp<VKPipeline> PipelineFactoryVulkan::build()
{
    setupVertexDescriptions();
    setupDescriptorSetLayout();

    VkDescriptorSet descriptorSet = setupDescriptorSet();
    VkPipeline pipeline = createPipeline();

    return sp<VKPipeline>::make(_resource_manager->recycler(), _renderer->renderTarget(), _pipeline_layout, _descriptor_set_layout, descriptorSet, pipeline);
}

sp<Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& /*graphicsContext*/, const PipelineLayout& pipelineLayout, const ShaderBindings& bindings)
{
    const bytearray ubo = sp<DynamicArray<uint8_t>>::make(bindings.pipelineInput()->ubo()->size());

    setupVertexDescriptions(pipelineLayout.input());
    setupDescriptorSetLayout();

    VkDescriptorSet descriptorSet = setupDescriptorSet(ubo, bindings);
    VkPipeline pipeline = createPipeline();

    return sp<VKPipeline>::make(_resource_manager->recycler(), _renderer->renderTarget(), _pipeline_layout, _descriptor_set_layout, descriptorSet, pipeline);
}

void PipelineFactoryVulkan::setupVertexDescriptions(const PipelineInput& input)
{
    vertices.bindingDescriptions.clear();
    vertices.attributeDescriptions.clear();

    for(const auto& iter : input.streams())
    {
        uint32_t divsor = iter.first;
        const PipelineInput::Stream& stream = iter.second;
        vertices.bindingDescriptions.push_back(vks::initializers::vertexInputBindingDescription(
                                                   divsor,
                                                   stream.stride(),
                                                   divsor == 0 ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE));

        uint32_t location = 0;
        for(const auto& i : stream.attributes().values())
        {
            vertices.attributeDescriptions.push_back(vks::initializers::vertexInputAttributeDescription(
                                                         divsor,
                                                         location,
                                                         getFormat(i),
                                                         i.offset()));
            _location_map[i.name()] = (location++);
        }
    }
}

void PipelineFactoryVulkan::setupVertexDescriptions()
{
    // Binding description
    vertices.bindingDescriptions.resize(1);
    vertices.bindingDescriptions[0] =
            vks::initializers::vertexInputBindingDescription(
                0,
                sizeof(VKUtil::Vertex),
                VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // Describes memory layout and shader positions
    vertices.attributeDescriptions.resize(3);
    // Location 0 : Position
    vertices.attributeDescriptions[0] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(VKUtil::Vertex, pos));
    // Location 1 : Texture coordinates
    vertices.attributeDescriptions[1] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                1,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(VKUtil::Vertex, uv));
    // Location 1 : Vertex normal
    vertices.attributeDescriptions[2] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                2,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(VKUtil::Vertex, normal));

    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void PipelineFactoryVulkan::setupDescriptorSetLayout()
{
    const sp<VKDevice>& device = _renderer->device();

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT,
        0),
        // Binding 1 : Fragment shader image sampler
        vks::initializers::descriptorSetLayoutBinding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        1)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                static_cast<uint32_t>(setLayoutBindings.size()));

    VKUtil::checkResult(vkCreateDescriptorSetLayout(device->logicalDevice(), &descriptorLayout, nullptr, &_descriptor_set_layout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &_descriptor_set_layout,
                1);

    VKUtil::checkResult(vkCreatePipelineLayout(device->logicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &_pipeline_layout));
}


VkDescriptorSet PipelineFactoryVulkan::setupDescriptorSet()
{
    const sp<VKDevice>& device = _renderer->device();

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                _renderer->vkDescriptorPool(),
                &_descriptor_set_layout,
                1);

    VKUtil::checkResult(vkAllocateDescriptorSets(device->logicalDevice(), &allocInfo, &descriptorSet));

    const VkDescriptorImageInfo imageDesc = _texture->descriptor();
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::writeDescriptorSet(
        descriptorSet,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        &_ubo->descriptor()),
        // Binding 1 : Fragment shader texture sampler
        //	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
        vks::initializers::writeDescriptorSet(
        descriptorSet,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// The descriptor set will use a combined image sampler (sampler and image could be split)
        1,												// Shader binding point 1
        &imageDesc)								        // Pointer to the descriptor image for our texture
    };

    vkUpdateDescriptorSets(device->logicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    return descriptorSet;
}

VkDescriptorSet PipelineFactoryVulkan::setupDescriptorSet(const bytearray& ubo, const ShaderBindings& shaderBindings)
{
    const sp<VKDevice>& device = _renderer->device();
    const sp<VKBuffer> uniform = sp<VKBuffer>::make(_renderer, _resource_manager->recycler(), nullptr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                _renderer->vkDescriptorPool(),
                &_descriptor_set_layout, 1);

    VKUtil::checkResult(vkAllocateDescriptorSets(device->logicalDevice(), &allocInfo, &descriptorSet));

    std::vector<VkDescriptorImageInfo> samplerDescriptors;
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::writeDescriptorSet(
        descriptorSet,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        &_ubo->descriptor())
    };

    uint32_t binding = 0;
    for(const sp<Texture>& i : shaderBindings.samplers())
    {
        const sp<VKTexture2D> texture = i->resource().as<VKTexture2D>();
        DASSERT(texture);
        writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                          descriptorSet,
                                          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                          ++binding,
                                          &samplerDescriptors.back()));
    }

    vkUpdateDescriptorSets(device->logicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    return descriptorSet;
}

VkPipeline PipelineFactoryVulkan::createPipeline()
{
    const sp<VKDevice>& device = _renderer->device();

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            vks::initializers::pipelineInputAssemblyStateCreateInfo(
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                0,
                VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
            vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_FRONT_BIT,
                VK_FRONT_FACE_COUNTER_CLOCKWISE,
                0);

    VkPipelineColorBlendAttachmentState blendAttachmentState =
            vks::initializers::pipelineColorBlendAttachmentState(
                0xf,
                VK_TRUE);
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState =
            vks::initializers::pipelineColorBlendStateCreateInfo(
                1,
                &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
            vks::initializers::pipelineDepthStencilStateCreateInfo(
                VK_TRUE,
                VK_TRUE,
                VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
            vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
            vks::initializers::pipelineMultisampleStateCreateInfo(
                VK_SAMPLE_COUNT_1_BIT,
                0);

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
            vks::initializers::pipelineDynamicStateCreateInfo(
                dynamicStateEnables.data(),
                static_cast<uint32_t>(dynamicStateEnables.size()),
                0);

    // Load shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

//    shaderStages[0] = VulkanAPI::loadShaderSPIR(_device->logicalDevice(), "texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//    shaderStages[1] = VulkanAPI::loadShaderSPIR(_device->logicalDevice(), "texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    shaderStages[0] = VKUtil::loadShader(device->logicalDevice(), "texture.vert", VKUtil::SHADER_TYPE_VERTEX);
    shaderStages[1] = VKUtil::loadShader(device->logicalDevice(), "texture.frag", VKUtil::SHADER_TYPE_FRAGMENT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
            vks::initializers::pipelineCreateInfo(
                _pipeline_layout,
                _renderer->vkRenderPass(),
                0);

    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VKUtil::checkResult(vkCreateGraphicsPipelines(device->logicalDevice(), device->pipelineCache(), 1, &pipelineCreateInfo, nullptr, &pipeline));
    vkDestroyShaderModule(device->logicalDevice(), shaderStages[0].module, nullptr);
    vkDestroyShaderModule(device->logicalDevice(), shaderStages[1].module, nullptr);

    return pipeline;
}

VkFormat PipelineFactoryVulkan::getFormat(const Attribute& attribute) const
{
    if(attribute.type() == Attribute::TYPE_FLOAT)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
            return formats[attribute.length() - 1];
        }
    }
    else if(attribute.type() == Attribute::TYPE_INTEGER)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT};
            return formats[attribute.length() - 1];
        }
    }
    else if(attribute.type() == Attribute::TYPE_SHORT)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16A16_SINT};
            return formats[attribute.length() - 1];
        }
    }
    else if(attribute.type() == Attribute::TYPE_USHORT)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16A16_UINT};
            return formats[attribute.length() - 1];
        }
    }
    DFATAL("Unsupport type %s, length %d", attribute.declareType().c_str(), attribute.length());
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

}
}
