#include "renderer/vulkan/base/vk_pipeline.h"

#include "core/base/observer.h"
#include "core/impl/uploader/uploader_array.h"

#include "renderer/base/buffer.h"
#include "renderer/base/compute_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/recycler.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/util/render_util.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_compute_context.h"
#include "renderer/vulkan/base/vk_descriptor_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_swap_chain.h"
#include "renderer/vulkan/base/vk_texture.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark::vulkan {

class VKPipeline::BakedRenderer {
public:
    virtual ~BakedRenderer() = default;

    virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) = 0;
};

namespace {

struct VKDrawArrays final : VKPipeline::BakedRenderer {
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override
    {
        const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
        vkCmdDraw(commandBuffer, drawingContext._draw_count, 1, param._start, 0);
    }
};

struct VKDrawElements final : VKPipeline::BakedRenderer {
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override
    {
        const DrawingParams::DrawElements& param = drawingContext._parameters.drawElements();
        vkCmdDrawIndexed(commandBuffer, drawingContext._draw_count, 1, param._start, 0, 0);
    }
};

struct VKDrawElementsInstanced final : VKPipeline::BakedRenderer {
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override
    {
        VkDeviceSize offsets = 0;
        const DrawingParams::DrawElementsInstanced& param = drawingContext._parameters.drawElementsInstanced();
        for(const auto& i : param._divided_buffer_snapshots)
        {
            i.second.upload(graphicsContext);
            DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
            VkBuffer vkInstanceVertexBuffer = (VkBuffer) (i.second.id());
            vkCmdBindVertexBuffers(commandBuffer, i.first, 1, &vkInstanceVertexBuffer, &offsets);
        }
        vkCmdDrawIndexed(commandBuffer, param._count, drawingContext._draw_count, param._start, 0, 0);
    }
};

struct VKMultiDrawElementsIndirect final : VKPipeline::BakedRenderer {
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override
    {
        VkDeviceSize offsets = 0;
        const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();
        for(const auto& [i, j] : param._divided_buffer_snapshots)
        {
            j.upload(graphicsContext);
            DCHECK(j.id(), "Invaild Instanced Array Buffer: %d", i);
            VkBuffer vkInstanceVertexBuffer = (VkBuffer) (j.id());
            vkCmdBindVertexBuffers(commandBuffer, i, 1, &vkInstanceVertexBuffer, &offsets);
        }

        param._indirect_cmds.upload(graphicsContext);
        vkCmdDrawIndexedIndirect(commandBuffer, (VkBuffer) (param._indirect_cmds.id()), 0, param._indirect_cmd_count, sizeof(DrawingParams::DrawElementsIndirectCommand));
    }
};

sp<VKPipeline::BakedRenderer> makeBakedRenderer(const PipelineBindings& bindings)
{
    switch(bindings.drawProcedure())
    {
        case Enum::DRAW_PROCEDURE_DRAW_ARRAYS:
            return sp<VKDrawArrays>::make();
        case Enum::DRAW_PROCEDURE_DRAW_ELEMENTS:
            return sp<VKDrawElements>::make();
        case Enum::DRAW_PROCEDURE_DRAW_INSTANCED:
            return sp<VKDrawElementsInstanced>::make();
        case Enum::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT:
            return sp<VKMultiDrawElementsIndirect>::make();
    }
    DFATAL("Not render procedure creator for %d", bindings.drawProcedure());
    return nullptr;
}

}

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

void VKPipeline::upload(GraphicsContext& graphicsContext)
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

ResourceRecycleFunc VKPipeline::recycle()
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
            _rebind_needed = true;

    if(_rebind_needed)
        setupDescriptorSet(graphicsContext, drawingContext._pipeline_context._shader_bindings->pipelineBindings());

    bindUBOShapshots(graphicsContext, drawingContext._pipeline_context._ubos);
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
    bindUBOShapshots(graphicsContext, computeContext._pipeline_context._ubos);
    buildComputeCommandBuffer(graphicsContext, computeContext);
}

void VKPipeline::setupVertexDescriptions(const PipelineInput& input, VKPipeline::VertexLayout& vertexLayout)
{
    uint32_t location = 0;
    for(const auto& [divsor, stream] : input.layouts())
    {
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
            stages = stages | VKUtil::toStage(j);

        binding = std::max(binding, i._binding);
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stages, i._binding));
    }

    for(size_t i = 0; i < pipelineInput.samplerCount(); ++i)
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, ++binding));

    for(size_t i = 0; i < pipelineInput.imageNames().size(); ++i)
        setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_VERTEX_BIT, ++binding));

    const VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                static_cast<uint32_t>(setLayoutBindings.size()));

    VKUtil::checkResult(vkCreateDescriptorSetLayout(device->vkLogicalDevice(), &descriptorLayout, nullptr, &_descriptor_set_layout));

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
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
        sp<VKBuffer> ubo = sp<VKBuffer>::make(_renderer, _recycler, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        ubo->uploadBuffer(graphicsContext, sp<UploaderArray<uint8_t>>::make(std::vector<uint8_t>(i->size(), 0)));
        binding = std::max(binding, i->binding());
        writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                          _descriptor_set,
                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          i->binding(),
                                          &ubo->vkDescriptor()));
        _ubos.push_back(std::move(ubo));
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
    for(const sp<Texture>& i : bindings.samplers().values())
    {
        DCHECK_WARN(i, "Pipeline has unbound sampler");
        if(i)
        {
            const sp<VKTexture> texture = i->delegate();
            _texture_observers.push_back(texture->observer().addBooleanSignal());
            if(texture->vkDescriptor().imageView && texture->vkDescriptor().imageLayout)
                writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                              _descriptor_set,
                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                              ++binding,
                                              &texture->vkDescriptor()));
        }
    }
    for(const sp<Texture>& i : bindings.images())
    {
        DCHECK_WARN(i, "Pipeline has unbound image");
        if(i)
        {
            const sp<VKTexture> texture = i->delegate();
            _texture_observers.push_back(texture->observer().addBooleanSignal());
            if(texture->vkDescriptor().imageView && texture->vkDescriptor().imageLayout)
                writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                              _descriptor_set,
                                              VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
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

    VkPipelineRasterizationStateCreateInfo rasterizationState = makeRasterizationState();

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

    CHECK_WARN(blendAttachmentStates.size() > 0, "Graphics pipeline has no color attachment");

    VkPipelineColorBlendStateCreateInfo colorBlendState =
            vks::initializers::pipelineColorBlendStateCreateInfo(
                static_cast<uint32_t>(blendAttachmentStates.size()),
                blendAttachmentStates.data());

    VkPipelineDepthStencilStateCreateInfo depthStencilState = makeDepthStencilState();

    const RenderEngineContext::Resolution& displayResolution = graphicsContext.renderContext()->displayResolution();
    const Optional<Rect>& scissor = _bindings.scissor();
    const VkRect2D vkScissors = scissor ? VkRect2D({{static_cast<int32_t>(scissor->left()), static_cast<int32_t>(scissor->top())}, {static_cast<uint32_t>(scissor->width()), static_cast<uint32_t>(scissor->height())}})
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
    for(const auto& [k, v] : _shaders)
        shaderStages.push_back(VKUtil::createShader(device->vkLogicalDevice(), v, k));

    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.attachments().ensure<VKGraphicsContext>();
    VKGraphicsContext::State& state = vkGraphicsContext->getCurrentState();
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(_layout, state.createRenderPass(_bindings), 0);

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
    VKGraphicsContext::State& state = vkGraphicsContext->getCurrentState();
    VkCommandBuffer commandBuffer = state.startRecording();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _layout, 0, 1, &_descriptor_set, 0, nullptr);

    VkDeviceSize offsets = 0;
    VkBuffer vkVertexBuffer = (VkBuffer)(drawingContext._vertices.id());
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &offsets);
    if(drawingContext._indices)
        vkCmdBindIndexBuffer(commandBuffer, (VkBuffer)(drawingContext._indices.id()), 0, kVKIndexType);

    if(const Optional<Rect>& scissor = drawingContext._scissor)
    {
        CHECK(drawingContext._pipeline_context._shader_bindings->pipelineBindings()->hasFlag(PipelineBindings::FLAG_DYNAMIC_SCISSOR, PipelineBindings::FLAG_DYNAMIC_SCISSOR_BITMASK), "Pipeline has no DYNAMIC_SCISSOR flag set");
        VkRect2D vkScissor{{static_cast<int32_t>(scissor->left()), static_cast<int32_t>(scissor->top())}, {static_cast<uint32_t>(scissor->width()), static_cast<uint32_t>(scissor->height())}};
        vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
    }

    _baked_renderer->draw(graphicsContext, drawingContext, commandBuffer);
}

void VKPipeline::buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    const sp<VKComputeContext>& vkContext = graphicsContext.attachments().ensure<VKComputeContext>();

    VkCommandBuffer commandBuffer = vkContext->buildCommandBuffer(graphicsContext);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _layout, 0, 1, &_descriptor_set, 0, nullptr);
    vkCmdDispatch(commandBuffer, computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));
}

bool VKPipeline::isDirty(const ByteArray::Borrowed& dirtyFlags) const
{
    size_t size = dirtyFlags.length();
    const uint8_t* buf = dirtyFlags.buf();
    for(size_t i = 0; i < size; ++i)
        if(buf[i])
            return true;
    return false;
}

sp<VKDescriptorPool> VKPipeline::makeDescriptorPool() const
{
    std::map<VkDescriptorType, uint32_t> poolSizes;
    if(_bindings.input()->ubos().size())
        poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = static_cast<uint32_t>(_bindings.input()->ubos().size());
    if(_bindings.input()->ssbos().size())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = static_cast<uint32_t>(_bindings.input()->ssbos().size());
    if(_bindings.samplers().size() + _bindings.images().size())
        poolSizes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = static_cast<uint32_t>(_bindings.samplers().size() + _bindings.images().size());
    if(_bindings.images().size())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] = static_cast<uint32_t>(_bindings.images().size());
    return sp<VKDescriptorPool>::make(_recycler, _renderer->device(), std::move(poolSizes));
}

void VKPipeline::bindUBOShapshots(GraphicsContext& graphicsContext, const std::vector<RenderLayerSnapshot::UBOSnapshot>& uboSnapshots)
{
    DCHECK(uboSnapshots.size() == _ubos.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), _ubos.size());

    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderLayerSnapshot::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        if(_rebind_needed || isDirty(uboSnapshot._dirty_flags))
        {
            const sp<VKBuffer>& ubo = _ubos.at(i);
            ubo->reload(graphicsContext, uboSnapshot._buffer);
        }
    }
}

VkPipelineDepthStencilStateCreateInfo VKPipeline::makeDepthStencilState() const
{
    VkPipelineDepthStencilStateCreateInfo state{};
    state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    state.depthTestEnable = VK_TRUE;
    state.depthWriteEnable = VK_TRUE;
    state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    state.stencilTestEnable = false;

    if(_bindings.parameters()._traits.has(PipelineBindings::TRAIT_TYPE_DEPTH_TEST))
    {
        const PipelineBindings::TraitDepthTest& depthTest = _bindings.parameters()._traits.at(PipelineBindings::TRAIT_TYPE_DEPTH_TEST)._configure._depth_test;
        state.depthTestEnable = depthTest._enabled;
        state.depthWriteEnable = depthTest._write_enabled;
        state.depthCompareOp = VKUtil::toCompareOp(depthTest._func);
    }

    if(_bindings.parameters()._traits.has(PipelineBindings::TRAIT_TYPE_STENCIL_TEST))
    {
        state.stencilTestEnable = true;
        const PipelineBindings::TraitStencilTest& stencilTest = _bindings.parameters()._traits.at(PipelineBindings::TRAIT_TYPE_STENCIL_TEST)._configure._stencil_test;
        if(stencilTest._front._type == PipelineBindings::FRONT_FACE_TYPE_DEFAULT && stencilTest._front._type == stencilTest._back._type)
            state.front = state.back = makeStencilState(stencilTest._front);
        else
        {
            if(stencilTest._front._type == PipelineBindings::FRONT_FACE_TYPE_FRONT)
                state.front = makeStencilState(stencilTest._front);
            if(stencilTest._back._type == PipelineBindings::FRONT_FACE_TYPE_BACK)
                state.back = makeStencilState(stencilTest._back);
        }
    }
    return state;
}

VkStencilOpState VKPipeline::makeStencilState(const PipelineBindings::TraitStencilTestSeparate& stencil) const
{
    VkStencilOpState state{};
    state.failOp = VKUtil::toStencilOp(stencil._op);
    state.passOp = VKUtil::toStencilOp(stencil._op_dpass);
    state.depthFailOp = VKUtil::toStencilOp(stencil._op_dfail);
    state.compareOp = VKUtil::toCompareOp(stencil._func);
    state.reference = stencil._ref;
    state.writeMask = stencil._mask;
    state.compareMask = stencil._compare_mask;
    return state;
}

VkPipelineRasterizationStateCreateInfo VKPipeline::makeRasterizationState() const
{
    if(_bindings.parameters()._traits.has(PipelineBindings::TRAIT_TYPE_CULL_FACE_TEST))
    {
        const PipelineBindings::TraitCullFaceTest& cullFaceTest = _bindings.parameters()._traits.at(PipelineBindings::TRAIT_TYPE_CULL_FACE_TEST)._configure._cull_face_test;
        return vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL, cullFaceTest._enabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE,
                VKUtil::toFrontFace(cullFaceTest._front_face), 0);
    }
    const VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // const VkFrontFace frontFace = Ark::instance().renderController()->renderEngine()->isLHS() ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    return vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, frontFace, 0);
}

}
