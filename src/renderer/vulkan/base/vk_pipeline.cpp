#include "renderer/vulkan/base/vk_pipeline.h"

#include "core/base/observer.h"

#include "renderer/base/buffer.h"
#include "renderer/base/compute_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/recycler.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/uploader.h"
#include "renderer/util/render_util.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_compute_context.h"
#include "renderer/vulkan/base/vk_descriptor_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKPipeline::VKPipeline(const PipelineBindings& bindings, const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, std::map<PipelineInput::ShaderStage, String> shaders)
    : _bindings(bindings), _recycler(recycler), _renderer(renderer), _baked_renderer(makeBakedRenderer(bindings)), _layout(VK_NULL_HANDLE), _descriptor_set_layout(VK_NULL_HANDLE),
      _descriptor_set(VK_NULL_HANDLE), _pipeline(VK_NULL_HANDLE), _shaders(std::move(shaders)), _rebind_needed(true), _is_compute_pipeline(false)
{
    for(const auto& i : _shaders)
    {
        if(i.first == PipelineInput::SHADER_STAGE_COMPUTE)
        {
            _is_compute_pipeline = true;
            DCHECK(_shaders.size() == 1, "Compute stage is exclusive");
        }
    }
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
    setupDescriptorSetLayout(_bindings.input());

    _descriptor_pool = makeDescriptorPool();
    _descriptor_pool->upload(graphicsContext);

    setupDescriptorSet(graphicsContext, _bindings);

    if(_is_compute_pipeline)
        setupComputePipeline(graphicsContext);
    else
    {
        VertexLayout vertexLayout;
        setupVertexDescriptions(_bindings.input(), vertexLayout);
        setupGraphicsPipeline(graphicsContext, vertexLayout);
    }
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
    for(const sp<Boolean>& i : _texture_observers)
        if(i->val())
        {
            setupDescriptorSet(graphicsContext, drawingContext._shader_bindings->pipelineBindings());
            _rebind_needed = true;
            break;
        }

    bindUBOShapshots(graphicsContext, drawingContext._ubos);
    _rebind_needed = false;
}

void VKPipeline::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    DCHECK(!_is_compute_pipeline, "Not a drawing pipeline");
    buildDrawCommandBuffer(graphicsContext, drawingContext);
}

void VKPipeline::compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    DCHECK(_is_compute_pipeline, "Not a compute pipeline");
    bindUBOShapshots(graphicsContext, computeContext._ubos);
    buildComputeCommandBuffer(graphicsContext, computeContext);
}

void VKPipeline::setupVertexDescriptions(const PipelineInput& input, VKPipeline::VertexLayout& vertexLayout)
{
    uint32_t location = 0;
    for(const auto& i : input.streams())
    {
        uint32_t divsor = i.first;
        const PipelineInput::Stream& stream = i.second;
        vertexLayout.bindingDescriptions.push_back(vks::initializers::vertexInputBindingDescription(
                                                   divsor,
                                                   stream.stride(),
                                                   divsor == 0 ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE));

        for(const Attribute& j : stream.attributes().values())
        {
            uint32_t slen = std::min<uint32_t>(4, j.length());
            for(uint32_t offset = 0; offset < j.length(); offset += slen)
                vertexLayout.attributeDescriptions.push_back(vks::initializers::vertexInputAttributeDescription(
                                                             divsor,
                                                             location++,
                                                             VKUtil::toAttributeFormat(j.type(), std::min<uint32_t>(4, j.length() - offset)),
                                                             j.offset() + offset * j.componentSize()));
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
        for(PipelineInput::ShaderStage j : i->stages())
            stages = static_cast<VkShaderStageFlags>(stages | VKUtil::toStage(j));

        binding = std::max(binding, i->binding());
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stages, i->binding()));
    }
    for(const PipelineInput::SSBO& i : pipelineInput.ssbos())
    {
        VkShaderStageFlags stages = i._stages.empty() ? VK_SHADER_STAGE_ALL : static_cast<VkShaderStageFlags>(0);
        for(PipelineInput::ShaderStage j : i._stages)
            stages = static_cast<VkShaderStageFlags>(stages | VKUtil::toStage(j));

        binding = std::max(binding, i._binding);
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stages, i._binding));
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

void VKPipeline::setupDescriptorSet(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
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
    for(const sp<PipelineInput::UBO>& i : bindings.input()->ubos())
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
                                          &ubo->vkDescriptor()));
    }
    for(const PipelineInput::SSBO& i : bindings.input()->ssbos())
    {
        binding = std::max(binding, i._binding);
        const sp<VKBuffer> sbo = i._buffer.delegate();
        writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                          _descriptor_set,
                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                          i._binding,
                                          &sbo->vkDescriptor()));
    }

    _texture_observers.clear();
    for(const sp<Texture>& i : bindings.samplers())
    {
        DWARN(i, "Pipeline has unbound sampler");
        if(i)
        {
            const sp<VKTexture> texture = i->delegate();
            _texture_observers.push_back(i->notifier().createDirtyFlag());
            writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                          _descriptor_set,
                                          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                          ++binding,
                                          &texture->vkDescriptor()));
        }
    }

    vkUpdateDescriptorSets(device->vkLogicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VKPipeline::setupGraphicsPipeline(GraphicsContext& graphicsContext, const VertexLayout& vertexLayout)
{
    const sp<VKDevice>& device = _renderer->device();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            vks::initializers::pipelineInputAssemblyStateCreateInfo(
                VKUtil::toPrimitiveTopology(_bindings.mode()),
                0,
                VK_FALSE);

    const VkCullModeFlags cullModeFlags[] = {VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT, VK_CULL_MODE_NONE};
    VkPipelineRasterizationStateCreateInfo rasterizationState =
            vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL,
                cullModeFlags[_bindings.getFlag(PipelineBindings::FLAG_CULL_MODE_BITMASK)],
                VK_FRONT_FACE_COUNTER_CLOCKWISE,
                0);

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
    for(uint32_t i = 0; i < _bindings.layout()->colorAttachmentCount(); ++i)
    {
        VkPipelineColorBlendAttachmentState state = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_TRUE);
        state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        state.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
        state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        state.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentStates.push_back(state);
    }

    DWARN(blendAttachmentStates.size() > 0, "Graphics pipeline has no color attachment");

    VkPipelineColorBlendStateCreateInfo colorBlendState =
            vks::initializers::pipelineColorBlendStateCreateInfo(
                static_cast<uint32_t>(blendAttachmentStates.size()),
                blendAttachmentStates.data());

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
            vks::initializers::pipelineDepthStencilStateCreateInfo(
                VK_TRUE,
                VK_TRUE,
                VK_COMPARE_OP_LESS_OR_EQUAL);

    const RenderEngineContext::Resolution& displayResolution = graphicsContext.renderContext()->displayResolution();
    const Rect& scissor = _bindings.scissor();
    const VkRect2D vkScissors = RenderUtil::isScissorEnabled(scissor) ? VkRect2D({{static_cast<int32_t>(scissor.left()), static_cast<int32_t>(scissor.top())}, {static_cast<uint32_t>(scissor.width()), static_cast<uint32_t>(scissor.height())}})
                                                                      : VkRect2D({{0, 0}, {displayResolution.width, displayResolution.height}});
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    viewportState.pScissors = &vkScissors;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampleState =
            vks::initializers::pipelineMultisampleStateCreateInfo(
                VK_SAMPLE_COUNT_1_BIT,
                0);

    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT };

    if(_bindings.hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK))
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicState =
            vks::initializers::pipelineDynamicStateCreateInfo(
                dynamicStateEnables.data(),
                static_cast<uint32_t>(dynamicStateEnables.size()),
                0);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for(const auto& i : _shaders)
        shaderStages.push_back(VKUtil::createShader(device->vkLogicalDevice(), i.second, i.first));

    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.attachments().ensure<VKGraphicsContext>();
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(_layout, vkGraphicsContext->vkRenderPass(), 0);

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

void VKPipeline::setupComputePipeline(GraphicsContext& /*graphicsContext*/)
{
    const sp<VKDevice>& device = _renderer->device();
    VkPipelineShaderStageCreateInfo stage = VKUtil::createShader(device->vkLogicalDevice(), _shaders.begin()->second, _shaders.begin()->first);
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(_layout, 0);
    computePipelineCreateInfo.stage = stage;
    vkCreateComputePipelines(device->vkLogicalDevice(), device->vkPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &_pipeline);
    vkDestroyShaderModule(device->vkLogicalDevice(), stage.module, nullptr);
}

void VKPipeline::buildDrawCommandBuffer(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.attachments().ensure<VKGraphicsContext>();

    VkCommandBuffer commandBuffer = vkGraphicsContext->vkCommandBuffer();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _layout, 0, 1, &_descriptor_set, 0, nullptr);

    VkDeviceSize offsets = 0;
    VkBuffer vkVertexBuffer = (VkBuffer)(drawingContext._vertex_buffer.id());
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &offsets);
    if(drawingContext._index_buffer)
        vkCmdBindIndexBuffer(commandBuffer, (VkBuffer)(drawingContext._index_buffer.id()), 0, kVKIndexType);

    const Rect& scissor = drawingContext._scissor;
    if(scissor.right() > scissor.left() && scissor.bottom() >= scissor.top())
    {
        DCHECK(drawingContext._shader_bindings->pipelineBindings()->hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK), "Pipeline has no DYNAMIC_SCISSOR flag set");
        VkRect2D vkScissor{{static_cast<int32_t>(scissor.left()), static_cast<int32_t>(scissor.top())}, {static_cast<uint32_t>(scissor.width()), static_cast<uint32_t>(scissor.height())}};
        vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
    }

    _baked_renderer->draw(graphicsContext, drawingContext, commandBuffer);
}

void VKPipeline::buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    const sp<VKComputeContext>& vkContext = graphicsContext.attachments().ensure<VKComputeContext>();

    VkCommandBuffer commandBuffer = vkContext->start();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _layout, 0, 1, &_descriptor_set, 0, nullptr);
    vkCmdDispatch(commandBuffer, computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));
}

bool VKPipeline::isDirty(const ByteArray::Borrowed& dirtyFlags) const
{
    size_t size = dirtyFlags.length();
    uint8_t* buf = dirtyFlags.buf();
    for(size_t i = 0; i < size; ++i)
        if(buf[i])
            return true;
    return false;
}

sp<VKPipeline::BakedRenderer> VKPipeline::makeBakedRenderer(const PipelineBindings& bindings) const
{
    switch(bindings.renderProcedure())
    {
        case PipelineBindings::RENDER_PROCEDURE_DRAW_ARRAYS:
            return sp<VKDrawArrays>::make();
        case PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS:
            return sp<VKDrawElements>::make();
        case PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS_INSTANCED:
            DASSERT(bindings.hasDivisors());
            return sp<VKDrawElementsInstanced>::make();
        case PipelineBindings::RENDER_PROCEDURE_DRAW_MULTI_ELEMENTS_INDIRECT:
            return sp<VKMultiDrawElementsIndirect>::make();
        }
    DFATAL("Not render procedure creator for %d", bindings.renderProcedure());
    return nullptr;
}

sp<VKDescriptorPool> VKPipeline::makeDescriptorPool() const
{
    std::map<VkDescriptorType, uint32_t> poolSizes;
    if(_bindings.input()->ubos().size())
        poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = static_cast<uint32_t>(_bindings.input()->ubos().size());
    if(_bindings.input()->ssbos().size())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = static_cast<uint32_t>(_bindings.input()->ssbos().size());
    if(_bindings.samplers().size())
        poolSizes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = static_cast<uint32_t>(_bindings.samplers().size());
    return sp<VKDescriptorPool>::make(_recycler, _renderer->device(), std::move(poolSizes));
}

void VKPipeline::bindUBOShapshots(GraphicsContext& graphicsContext, const std::vector<RenderLayer::UBOSnapshot>& uboSnapshots)
{
    DCHECK(uboSnapshots.size() == _ubos.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), _ubos.size());

    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderLayer::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        if(_rebind_needed || isDirty(uboSnapshot._dirty_flags))
        {
            const sp<VKBuffer>& ubo = _ubos.at(i);
            ubo->reload(graphicsContext, uboSnapshot._buffer);
        }
    }
}

void VKPipeline::VKDrawArrays::draw(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer)
{
    const DrawingContext::ParamDrawElements& param = drawingContext._parameters._draw_elements;
    DASSERT(param.isActive());
    vkCmdDraw(commandBuffer, param._count, 1, param._start, 0);
}

void VKPipeline::VKDrawElements::draw(GraphicsContext& /*graphicsContext*/, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer)
{
    const DrawingContext::ParamDrawElements& param = drawingContext._parameters._draw_elements;
    DASSERT(param.isActive());
    vkCmdDrawIndexed(commandBuffer, param._count, 1, param._start, 0, 0);
}

void VKPipeline::VKDrawElementsInstanced::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer)
{
    const DrawingContext::ParamDrawElementsInstanced& param = drawingContext._parameters._draw_elements_instanced;
    DASSERT(param.isActive());

    VkDeviceSize offsets = 0;
    for(const auto& i : param._instanced_array_snapshots)
    {
        i.second.upload(graphicsContext);
        DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
        VkBuffer vkInstanceVertexBuffer = (VkBuffer) (i.second.id());
        vkCmdBindVertexBuffers(commandBuffer, i.first, 1, &vkInstanceVertexBuffer, &offsets);
    }
    vkCmdDrawIndexed(commandBuffer, param._count, static_cast<uint32_t>(param._instance_count), param._start, 0, 0);
}

void VKPipeline::VKMultiDrawElementsIndirect::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer)
{
    const DrawingContext::ParamDrawMultiElementsIndirect& param = drawingContext._parameters._draw_multi_elements_indirect;
    DASSERT(param.isActive());

    VkDeviceSize offsets = 0;
    for(const auto& i : param._instanced_array_snapshots)
    {
        i.second.upload(graphicsContext);
        DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
        VkBuffer vkInstanceVertexBuffer = (VkBuffer) (i.second.id());
        vkCmdBindVertexBuffers(commandBuffer, i.first, 1, &vkInstanceVertexBuffer, &offsets);
    }

    param._indirect_cmds.upload(graphicsContext);
    vkCmdDrawIndexedIndirect(commandBuffer, (VkBuffer) (param._indirect_cmds.id()), 0, param._draw_count, sizeof(DrawingContext::DrawElementsIndirectCommand));
}

}
}
