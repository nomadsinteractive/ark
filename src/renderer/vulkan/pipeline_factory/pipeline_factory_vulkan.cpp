#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include <array>

#include "renderer/base/drawing_context.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/device.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/render_target.h"
#include "renderer/vulkan/base/vk_texture.h"
#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<ResourceManager>& resourceManager)
    : _resource_manager(resourceManager) {
}

PipelineFactoryVulkan::~PipelineFactoryVulkan()
{
    if(_device)
        vkDestroyDescriptorPool(_device->logicalDevice(), descriptorPool, nullptr);
}

sp<VKPipeline> PipelineFactoryVulkan::build(const sp<RenderTarget>& renderTarget)
{
    _render_target = renderTarget;
    _device = renderTarget->device();
    setupVertexDescriptions();
    setupDescriptorSetLayout();
    setupDescriptorPool();
    preparePipelines();
    setupDescriptorSet();

    return sp<VKPipeline>::make(_render_target, pipelineLayout, descriptorSetLayout, descriptorSet, pipelines.solid);
}

sp<ark::Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& pipelineLayout)
{
    return nullptr;
}

sp<RenderCommand> PipelineFactoryVulkan::buildRenderCommand(ObjectPool& objectPool, DrawingContext drawingContext, const sp<Shader>& shader, RenderModel::Mode renderMode, int32_t count)
{
    return nullptr;
}

void PipelineFactoryVulkan::setupVertexDescriptions()
{
    // Binding description
    vertices.bindingDescriptions.resize(1);
    vertices.bindingDescriptions[0] =
            vks::initializers::vertexInputBindingDescription(
                0,
                sizeof(VulkanAPI::Vertex),
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
                offsetof(VulkanAPI::Vertex, pos));
    // Location 1 : Texture coordinates
    vertices.attributeDescriptions[1] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                1,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(VulkanAPI::Vertex, uv));
    // Location 1 : Vertex normal
    vertices.attributeDescriptions[2] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                2,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(VulkanAPI::Vertex, normal));

    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void PipelineFactoryVulkan::setupDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
    {
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

    VulkanAPI::checkResult(vkCreateDescriptorSetLayout(_device->logicalDevice(), &descriptorLayout, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &descriptorSetLayout,
                1);

    VulkanAPI::checkResult(vkCreatePipelineLayout(_device->logicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void PipelineFactoryVulkan::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
            vks::initializers::descriptorPoolCreateInfo(
                static_cast<uint32_t>(poolSizes.size()),
                poolSizes.data(),
                2);

    VulkanAPI::checkResult(vkCreateDescriptorPool(_device->logicalDevice(), &descriptorPoolInfo, nullptr, &descriptorPool));
}

void PipelineFactoryVulkan::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                descriptorPool,
                &descriptorSetLayout,
                1);

    VulkanAPI::checkResult(vkAllocateDescriptorSets(_device->logicalDevice(), &allocInfo, &descriptorSet));

    std::vector<VkWriteDescriptorSet> writeDescriptorSets =
    {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::writeDescriptorSet(
        descriptorSet,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        &_buffer->descriptor()),
        // Binding 1 : Fragment shader texture sampler
        //	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
        vks::initializers::writeDescriptorSet(
        descriptorSet,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// The descriptor set will use a combined image sampler (sampler and image could be split)
        1,												// Shader binding point 1
        &_texture->descriptor())								// Pointer to the descriptor image for our texture
    };

    vkUpdateDescriptorSets(_device->logicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void PipelineFactoryVulkan::preparePipelines()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            vks::initializers::pipelineInputAssemblyStateCreateInfo(
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                0,
                VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
            vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_NONE,
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

    shaderStages[0] = VulkanAPI::loadShaderSPIR(_device->logicalDevice(), "texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = VulkanAPI::loadShaderSPIR(_device->logicalDevice(), "texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
            vks::initializers::pipelineCreateInfo(
                pipelineLayout,
                _render_target->renderPass(),
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

    VulkanAPI::checkResult(vkCreateGraphicsPipelines(_device->logicalDevice(), _device->pipelineCache(), 1, &pipelineCreateInfo, nullptr, &pipelines.solid));
    vkDestroyShaderModule(_device->logicalDevice(), shaderStages[0].module, nullptr);
    vkDestroyShaderModule(_device->logicalDevice(), shaderStages[1].module, nullptr);
}

}
}
