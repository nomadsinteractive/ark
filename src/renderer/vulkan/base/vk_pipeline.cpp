#include "renderer/vulkan/base/vk_pipeline.h"

#include "renderer/base/buffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/recycler.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/ubo.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_descriptor_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

VKPipeline::VKPipeline(const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, const sp<ShaderBindings>& shaderBindings, std::map<Shader::Stage, String> shaders)
    : _recycler(recycler), _shader_bindings(shaderBindings), _renderer(renderer),
      _layout(VK_NULL_HANDLE), _descriptor_set_layout(VK_NULL_HANDLE), _descriptor_set(VK_NULL_HANDLE), _pipeline(VK_NULL_HANDLE),
      _shaders(std::move(shaders))
{
}

VKPipeline::~VKPipeline()
{
    _recycler->recycle(*this);
}

VkPipeline VKPipeline::vkPipeline() const
{
    return _pipeline;
}

VkPipelineLayout VKPipeline::vkPipelineLayout() const
{
    return _layout;
}

const VkDescriptorSet& VKPipeline::vkDescriptorSet() const
{
    return _descriptor_set;
}

uintptr_t VKPipeline::id()
{
    return reinterpret_cast<uintptr_t>(_pipeline);
}

void VKPipeline::upload()
{
    VertexLayout vertexLayout;

    setupVertexDescriptions(vertexLayout);
    setupDescriptorSetLayout();

    _descriptor_pool = _renderer->renderTarget()->makeDescriptorPool(sp<Recycler>::make());
    setupDescriptorSet();
    setupPipeline(vertexLayout);

}

void VKPipeline::upload(GraphicsContext& graphicsContext)
{
    VertexLayout vertexLayout;
    setupVertexDescriptions(_shader_bindings->pipelineInput(), vertexLayout);
    setupDescriptorSetLayout();

    _descriptor_pool = _renderer->renderTarget()->makeDescriptorPool(graphicsContext.resourceManager()->recycler());
    setupDescriptorSet(graphicsContext, _shader_bindings);
    setupPipeline(vertexLayout);
}

Resource::RecycleFunc VKPipeline::recycle()
{
    const sp<VKDevice> device = _renderer->device();

    VkPipelineLayout layout = _layout;
    VkDescriptorSetLayout descriptorSetLayout = _descriptor_set_layout;
    VkPipeline pipeline = _pipeline;

    return [device, layout, descriptorSetLayout, pipeline](GraphicsContext&) {
        if(layout)
            vkDestroyPipelineLayout(device->logicalDevice(), layout, nullptr);
        if(descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device->logicalDevice(), descriptorSetLayout, nullptr);
        if(pipeline)
            vkDestroyPipeline(device->logicalDevice(), pipeline, nullptr);
    };
}

sp<RenderCommand> VKPipeline::active(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext)
{
    if(!_command_buffers)
        buildCommandBuffers(drawingContext._array_buffer, drawingContext._index_buffer);

    return _command_buffers->aquire();
}

void VKPipeline::setupVertexDescriptions(const PipelineInput& input, VKPipeline::VertexLayout& vertexLayout)
{
    for(const auto& iter : input.streams())
    {
        uint32_t divsor = iter.first;
        const PipelineInput::Stream& stream = iter.second;
        vertexLayout.bindingDescriptions.push_back(vks::initializers::vertexInputBindingDescription(
                                                   divsor,
                                                   stream.stride(),
                                                   divsor == 0 ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE));

        uint32_t location = 0;
        for(const auto& i : stream.attributes().values())
        {
            vertexLayout.attributeDescriptions.push_back(vks::initializers::vertexInputAttributeDescription(
                                                         divsor,
                                                         location,
                                                         VKUtil::getAttributeFormat(i),
                                                         i.offset()));
            _location_map[i.name()] = (location++);
        }
    }
    vertexLayout.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertexLayout.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexLayout.bindingDescriptions.size());
    vertexLayout.inputState.pVertexBindingDescriptions = vertexLayout.bindingDescriptions.data();
    vertexLayout.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.attributeDescriptions.size());
    vertexLayout.inputState.pVertexAttributeDescriptions = vertexLayout.attributeDescriptions.data();
}

void VKPipeline::setupVertexDescriptions(VertexLayout& vertexLayout)
{
    // Binding description
    vertexLayout.bindingDescriptions.resize(1);
    vertexLayout.bindingDescriptions[0] =
            vks::initializers::vertexInputBindingDescription(
                0,
                sizeof(VKUtil::Vertex),
                VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // Describes memory layout and shader positions
    vertexLayout.attributeDescriptions.resize(3);
    // Location 0 : Position
    vertexLayout.attributeDescriptions[0] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(VKUtil::Vertex, pos));
    // Location 1 : Texture coordinates
    vertexLayout.attributeDescriptions[1] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                1,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(VKUtil::Vertex, uv));
    // Location 1 : Vertex normal
    vertexLayout.attributeDescriptions[2] =
            vks::initializers::vertexInputAttributeDescription(
                0,
                2,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(VKUtil::Vertex, normal));

    vertexLayout.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertexLayout.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexLayout.bindingDescriptions.size());
    vertexLayout.inputState.pVertexBindingDescriptions = vertexLayout.bindingDescriptions.data();
    vertexLayout.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.attributeDescriptions.size());
    vertexLayout.inputState.pVertexAttributeDescriptions = vertexLayout.attributeDescriptions.data();
}

void VKPipeline::setupDescriptorSetLayout()
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

    VKUtil::checkResult(vkCreatePipelineLayout(device->logicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &_layout));
}


void VKPipeline::setupDescriptorSet()
{
    const sp<VKDevice>& device = _renderer->device();

    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                _descriptor_pool->vkDescriptorPool(),
                &_descriptor_set_layout,
                1);

    VKUtil::checkResult(vkAllocateDescriptorSets(device->logicalDevice(), &allocInfo, &_descriptor_set));

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::writeDescriptorSet(
        _descriptor_set,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        &_ubo->descriptor()),
        // Binding 1 : Fragment shader texture sampler
        //	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
        vks::initializers::writeDescriptorSet(
        _descriptor_set,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// The descriptor set will use a combined image sampler (sampler and image could be split)
        1,												// Shader binding point 1
        &_texture->vkDescriptor())				        // Pointer to the descriptor image for our texture
    };

    vkUpdateDescriptorSets(device->logicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VKPipeline::setupDescriptorSet(GraphicsContext& graphicsContext, const ShaderBindings& shaderBindings)
{
    const sp<VKDevice>& device = _renderer->device();
    const sp<Uploader> uploader = sp<Uploader::Blank>::make(shaderBindings.pipelineInput()->ubo()->size());
    const sp<VKBuffer> ubo = sp<VKBuffer>::make(_renderer, _recycler, uploader,
                                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    ubo->upload(graphicsContext);

    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                _descriptor_pool->vkDescriptorPool(),
                &_descriptor_set_layout, 1);

    VKUtil::checkResult(vkAllocateDescriptorSets(device->logicalDevice(), &allocInfo, &_descriptor_set));

    std::vector<VkDescriptorImageInfo> samplerDescriptors;
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        vks::initializers::writeDescriptorSet(
        _descriptor_set,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        &ubo->descriptor())
    };

    uint32_t binding = 0;
    for(const sp<Texture>& i : shaderBindings.samplers())
    {
        const sp<VKTexture2D> texture = i->resource();
        samplerDescriptors.push_back(texture->vkDescriptor());
        writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                      _descriptor_set,
                                      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                      ++binding,
                                      &samplerDescriptors.back()));
    }

    vkUpdateDescriptorSets(device->logicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VKPipeline::setupPipeline(const VertexLayout& vertexLayout)
{
    const sp<VKDevice>& device = _renderer->device();

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
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

//    shaderStages[0] = VulkanAPI::loadShaderSPIR(_device->logicalDevice(), "texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//    shaderStages[1] = VulkanAPI::loadShaderSPIR(_device->logicalDevice(), "texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    if(_shaders.empty())
    {
        shaderStages.push_back(VKUtil::loadShader(device->logicalDevice(), "texture.vert", Shader::STAGE_VERTEX));
        shaderStages.push_back(VKUtil::loadShader(device->logicalDevice(), "texture.frag", Shader::STAGE_FRAGMENT));
    }
    else
    {
        for(const auto& iter : _shaders)
            shaderStages.push_back(VKUtil::createShader(device->logicalDevice(), iter.second, iter.first));
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
            vks::initializers::pipelineCreateInfo(
                _layout,
                _renderer->vkRenderPass(),
                0);

    pipelineCreateInfo.pVertexInputState = &vertexLayout.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VKUtil::checkResult(vkCreateGraphicsPipelines(device->logicalDevice(), device->pipelineCache(), 1, &pipelineCreateInfo, nullptr, &_pipeline));

    for(const auto& i : shaderStages)
        vkDestroyShaderModule(device->logicalDevice(), i.module, nullptr);
}

void VKPipeline::buildCommandBuffers(const Buffer::Snapshot& vertex, const Buffer::Snapshot& index)
{
    const sp<VKRenderTarget>& renderTarget = _renderer->renderTarget();

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkRenderPassBeginInfo renderPassBeginInfo = renderTarget->vkRenderPassBeginInfo();

    _command_buffers = sp<VKCommandBuffers>::make(_recycler, renderTarget);
    const std::vector<VkCommandBuffer>& commands = _command_buffers->vkCommandBuffers();
    for (size_t i = 0; i < commands.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = renderTarget->frameBuffers()[i];
        const VkCommandBuffer commandBuffer = _command_buffers->vkCommandBuffers()[i];
        VKUtil::checkResult(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(commandBuffer, 0, 1, &renderTarget->vkViewport());
        vkCmdSetScissor(commandBuffer, 0, 1, &renderTarget->vkScissor());

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _layout, 0, 1, &_descriptor_set, 0, nullptr);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

        VkDeviceSize offsets[1] = { 0 };

        VkBuffer vkVertexBuffer = reinterpret_cast<VkBuffer>(vertex.id());
        VkBuffer vkIndexBuffer = reinterpret_cast<VkBuffer>(index.id());

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, index.size() / sizeof(uint16_t), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
        VKUtil::checkResult(vkEndCommandBuffer(commandBuffer));
    }
}

}
}
