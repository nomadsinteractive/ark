#include "renderer/vulkan/base/vk_pipeline.h"

#include "core/base/observer.h"

#include "renderer/base/buffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/recycler.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_descriptor_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKPipeline::VKPipeline(const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, const sp<ShaderBindings>& shaderBindings, std::map<Shader::Stage, String> shaders)
    : _recycler(recycler), _shader_bindings(shaderBindings), _renderer(renderer), _layout(VK_NULL_HANDLE), _descriptor_set_layout(VK_NULL_HANDLE), _descriptor_set(VK_NULL_HANDLE),
      _pipeline(VK_NULL_HANDLE), _shaders(std::move(shaders))
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

uint64_t VKPipeline::id()
{
    return (uint64_t)(_pipeline);
}

void VKPipeline::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    VertexLayout vertexLayout;
    setupVertexDescriptions(_shader_bindings->pipelineInput(), vertexLayout);
    setupDescriptorSetLayout(_shader_bindings->pipelineInput());

    _descriptor_pool = _renderer->renderTarget()->makeDescriptorPool(graphicsContext.recycler());
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
            vkDestroyPipelineLayout(device->vkLogicalDevice(), layout, nullptr);
        if(descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device->vkLogicalDevice(), descriptorSetLayout, nullptr);
        if(pipeline)
            vkDestroyPipeline(device->vkLogicalDevice(), pipeline, nullptr);
    };
}

void VKPipeline::bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    bool reloadNeeded = false;
    for(const sp<Observer>& i : _texture_observers)
        if(i->dirty())
        {
            setupDescriptorSet(graphicsContext, drawingContext._shader_bindings);
            reloadNeeded = true;
            break;
        }

    const std::vector<RenderLayer::UBOSnapshot>& uboSnapshots = drawingContext._ubos;
    DCHECK(uboSnapshots.size() == _ubos.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), _ubos.size());

    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderLayer::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        if(reloadNeeded || isDirty(uboSnapshot._dirty_flags))
        {
            const sp<VKBuffer>& ubo = _ubos.at(i);
            ubo->reload(graphicsContext, uboSnapshot._buffer);
        }
    }
}

void VKPipeline::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    buildCommandBuffer(graphicsContext, drawingContext._vertex_buffer, drawingContext._index_buffer);
}

void VKPipeline::setupVertexDescriptions(const PipelineInput& input, VKPipeline::VertexLayout& vertexLayout)
{
    for(const auto& i : input.streams())
    {
        uint32_t divsor = i.first;
        const PipelineInput::Stream& stream = i.second;
        vertexLayout.bindingDescriptions.push_back(vks::initializers::vertexInputBindingDescription(
                                                   divsor,
                                                   stream.stride(),
                                                   divsor == 0 ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE));

        uint32_t location = 0;
        for(const auto& j : stream.attributes().values())
        {
            vertexLayout.attributeDescriptions.push_back(vks::initializers::vertexInputAttributeDescription(
                                                         divsor,
                                                         location++,
                                                         VKUtil::getAttributeFormat(j),
                                                         j.offset()));
        }
    }
    vertexLayout.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertexLayout.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexLayout.bindingDescriptions.size());
    vertexLayout.inputState.pVertexBindingDescriptions = vertexLayout.bindingDescriptions.data();
    vertexLayout.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.attributeDescriptions.size());
    vertexLayout.inputState.pVertexAttributeDescriptions = vertexLayout.attributeDescriptions.data();
}

void VKPipeline::setupDescriptorSetLayout(const PipelineInput& pipelineInput)
{
    const sp<VKDevice>& device = _renderer->device();

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    uint32_t binding = 0;
    for(const sp<PipelineInput::UBO>& i : pipelineInput.ubos())
    {
        VkShaderStageFlags stages = i->stages().empty() ? VK_SHADER_STAGE_ALL : static_cast<VkShaderStageFlags>(0);
        for(Shader::Stage j : i->stages())
            stages = static_cast<VkShaderStageFlags>(stages | VKUtil::toStage(j));

        binding = std::max(binding, i->binding());
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stages, i->binding()));
    }

    for(size_t i = 0; i < pipelineInput.samplerCount(); ++i)
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, ++binding));

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                static_cast<uint32_t>(setLayoutBindings.size()));

    VKUtil::checkResult(vkCreateDescriptorSetLayout(device->vkLogicalDevice(), &descriptorLayout, nullptr, &_descriptor_set_layout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &_descriptor_set_layout,
                1);

    VKUtil::checkResult(vkCreatePipelineLayout(device->vkLogicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &_layout));
}

void VKPipeline::setupDescriptorSet(GraphicsContext& graphicsContext, const ShaderBindings& shaderBindings)
{
    const sp<VKDevice>& device = _renderer->device();

    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                _descriptor_pool->vkDescriptorPool(),
                &_descriptor_set_layout, 1);

    VKUtil::checkResult(vkResetDescriptorPool(device->vkLogicalDevice(), _descriptor_pool->vkDescriptorPool(), 0));
    VKUtil::checkResult(vkAllocateDescriptorSets(device->vkLogicalDevice(), &allocInfo, &_descriptor_set));

    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    uint32_t binding = 0;

    _ubos.clear();
    for(const sp<PipelineInput::UBO>& i : shaderBindings.pipelineInput()->ubos())
    {
        const sp<Uploader> uploader = sp<Uploader::Blank>::make(i->size());
        const sp<VKBuffer> ubo = sp<VKBuffer>::make(_renderer, _recycler, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        ubo->upload(graphicsContext, uploader);
        _ubos.push_back(ubo);
        binding = std::max(binding, i->binding());
        writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                          _descriptor_set,
                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          i->binding(),
                                          &ubo->descriptor()));
    }

    _texture_observers.clear();
    for(const sp<Texture>& i : shaderBindings.samplers())
    {
        const sp<VKTexture2D> texture = i->resource();
        _texture_observers.push_back(i->notifier().createObserver(false));
        writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                      _descriptor_set,
                                      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                      ++binding,
                                      &texture->vkDescriptor()));
    }

    vkUpdateDescriptorSets(device->vkLogicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VKPipeline::setupPipeline(const VertexLayout& vertexLayout)
{
    const sp<VKDevice>& device = _renderer->device();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            vks::initializers::pipelineInputAssemblyStateCreateInfo(
                VKUtil::toPrimitiveTopology(_shader_bindings->renderMode()),
                0,
                VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
            vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_BACK_BIT,
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
    for(const auto& i : _shaders)
        shaderStages.push_back(VKUtil::createShader(device->vkLogicalDevice(), i.second, i.first));

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

    VKUtil::checkResult(vkCreateGraphicsPipelines(device->vkLogicalDevice(), device->vkPipelineCache(), 1, &pipelineCreateInfo, nullptr, &_pipeline));

    for(const auto& i : shaderStages)
        vkDestroyShaderModule(device->vkLogicalDevice(), i.module, nullptr);
}

void VKPipeline::buildCommandBuffer(GraphicsContext& graphicsContext, const Buffer::Snapshot& vertex, const Buffer::Snapshot& index)
{
    const sp<VKGraphicsContext>& vkContext = graphicsContext.attachment<VKGraphicsContext>();

    VkCommandBuffer commandBuffer = vkContext->vkCommandBuffer();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _layout, 0, 1, &_descriptor_set, 0, nullptr);

    VkBuffer vkVertexBuffer = (VkBuffer)(vertex.id());
    VkBuffer vkIndexBuffer = (VkBuffer)(index.id());

    VkDeviceSize offsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &offsets);
    vkCmdBindIndexBuffer(commandBuffer, vkIndexBuffer, 0, kVKIndexType);
    vkCmdDrawIndexed(commandBuffer, index.size() / sizeof(glindex_t), 1, 0, 0, 0);
}

bool VKPipeline::isDirty(const bytearray& dirtyFlags) const
{
    size_t size = dirtyFlags->length();
    uint8_t* buf = dirtyFlags->buf();
    for(size_t i = 0; i < size; ++i)
        if(buf[i])
            return true;
    return false;
}

}
}
