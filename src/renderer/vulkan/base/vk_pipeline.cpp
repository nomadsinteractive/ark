#include "renderer/vulkan/base/vk_pipeline.h"

#include "core/base/observer.h"
#include "core/impl/uploader/uploader_array.h"

#include "renderer/base/buffer.h"
#include "renderer/base/compute_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/util/render_util.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
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

sp<VKPipeline::BakedRenderer> makeBakedRenderer(const PipelineDescriptor& bindings)
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
        default:
            break;
    }
    DFATAL("Not render procedure creator for %d", bindings.drawProcedure());
    return nullptr;
}

VkStencilOpState makeStencilState(const PipelineDescriptor::TraitStencilTestSeparate& stencil)
{
    VkStencilOpState state = {};
    state.failOp = VKUtil::toStencilOp(stencil._op);
    state.passOp = VKUtil::toStencilOp(stencil._op_dpass);
    state.depthFailOp = VKUtil::toStencilOp(stencil._op_dfail);
    state.compareOp = VKUtil::toCompareOp(stencil._func);
    state.reference = stencil._ref;
    state.writeMask = stencil._mask;
    state.compareMask = stencil._compare_mask;
    return state;
}

bool isDirty(const ByteArray::Borrowed& dirtyFlags)
{
    const size_t size = dirtyFlags.length();
    const uint8_t* buf = dirtyFlags.buf();
    for(size_t i = 0; i < size; ++i)
        if(buf[i])
            return true;
    return false;
}

VkBlendFactor toBlendFactor(PipelineDescriptor::BlendFactor blendFactor)
{
    switch(blendFactor) {
        case PipelineDescriptor::BLEND_FACTOR_ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case PipelineDescriptor::BLEND_FACTOR_ONE:
            return VK_BLEND_FACTOR_ONE;
        case PipelineDescriptor::BLEND_FACTOR_SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_DST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_DST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case PipelineDescriptor::BLEND_FACTOR_CONST_COLOR:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case PipelineDescriptor::BLEND_FACTOR_CONST_ALPHA:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        default:
            break;
    }
    FATAL("Unknow BlendFacor: %d", blendFactor);
    return VK_BLEND_FACTOR_ZERO;
}

VkPipelineDepthStencilStateCreateInfo makeDepthStencilState(const PipelineDescriptor::PipelineTraitTable& traits)
{
    VkPipelineDepthStencilStateCreateInfo state = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    state.depthTestEnable = VK_TRUE;
    state.depthWriteEnable = VK_TRUE;
    state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    state.stencilTestEnable = false;

    if(traits.has(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST))
    {
        const PipelineDescriptor::TraitDepthTest& depthTest = traits.at(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)._configure._depth_test;
        state.depthTestEnable = depthTest._enabled;
        state.depthWriteEnable = depthTest._write_enabled;
        state.depthCompareOp = VKUtil::toCompareOp(depthTest._func);
    }

    if(traits.has(PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST))
    {
        state.stencilTestEnable = true;
        const PipelineDescriptor::TraitStencilTest& stencilTest = traits.at(PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST)._configure._stencil_test;
        if(stencilTest._front._type == PipelineDescriptor::FRONT_FACE_TYPE_DEFAULT && stencilTest._front._type == stencilTest._back._type)
            state.front = state.back = makeStencilState(stencilTest._front);
        else
        {
            if(stencilTest._front._type == PipelineDescriptor::FRONT_FACE_TYPE_FRONT)
                state.front = makeStencilState(stencilTest._front);
            if(stencilTest._back._type == PipelineDescriptor::FRONT_FACE_TYPE_BACK)
                state.back = makeStencilState(stencilTest._back);
        }
    }
    return state;
}

VkPipelineRasterizationStateCreateInfo makeRasterizationState(const PipelineDescriptor::PipelineTraitTable& traits)
{
    if(traits.has(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST))
    {
        const PipelineDescriptor::TraitCullFaceTest& cullFaceTest = traits.at(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST)._configure._cull_face_test;
        return vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL, cullFaceTest._enabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE,
                VKUtil::toFrontFace(cullFaceTest._front_face), 0);
    }
    return vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
}

VkPipelineColorBlendAttachmentState makeColorBlendAttachmentState(const PipelineDescriptor::PipelineTraitTable& traits)
{
    VkPipelineColorBlendAttachmentState cbaState = vks::initializers::pipelineColorBlendAttachmentState(0xf, true);
    cbaState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cbaState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cbaState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
    cbaState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cbaState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cbaState.colorBlendOp = VK_BLEND_OP_ADD;
    if(traits.has(PipelineDescriptor::TRAIT_TYPE_BLEND))
    {
        const PipelineDescriptor::PipelineTraitMeta& tm = traits.at(PipelineDescriptor::TRAIT_TYPE_BLEND);
        cbaState.blendEnable = tm._configure._blend._enabled;
        if(tm._configure._blend._src_rgb_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.srcColorBlendFactor = toBlendFactor(tm._configure._blend._src_rgb_factor);
        if(tm._configure._blend._dst_rgb_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.dstColorBlendFactor = toBlendFactor(tm._configure._blend._dst_rgb_factor);
        if(tm._configure._blend._src_alpha_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.srcAlphaBlendFactor = toBlendFactor(tm._configure._blend._src_alpha_factor);
        if(tm._configure._blend._dst_alpha_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.dstAlphaBlendFactor = toBlendFactor(tm._configure._blend._dst_alpha_factor);
    }
    return cbaState;
}

}

VKPipeline::VKPipeline(const PipelineDescriptor& bindings, const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, std::map<Enum::ShaderStageBit, String> stages)
    : _pipeline_descriptor(bindings), _recycler(recycler), _renderer(renderer), _baked_renderer(makeBakedRenderer(bindings)), _layout(VK_NULL_HANDLE), _descriptor_set_layout(VK_NULL_HANDLE),
      _descriptor_set(VK_NULL_HANDLE), _pipeline(VK_NULL_HANDLE), _stages(std::move(stages)), _rebind_needed(true), _is_compute_pipeline(false)
{
    for(const auto& i : _stages)
        if(i.first == Enum::SHADER_STAGE_BIT_COMPUTE)
        {
            _is_compute_pipeline = true;
            CHECK(_stages.size() == 1, "Compute stage is exclusive");
            break;
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
    setupDescriptorSetLayout(_pipeline_descriptor);

    _descriptor_pool = makeDescriptorPool();
    _descriptor_pool->upload(graphicsContext);

    setupDescriptorSet(graphicsContext, _pipeline_descriptor);

    if(_is_compute_pipeline)
        setupComputePipeline(graphicsContext);
    else
    {
        VertexLayout vertexLayout;
        setupVertexDescriptions(_pipeline_descriptor.input(), vertexLayout);
        setupGraphicsPipeline(graphicsContext, vertexLayout);
    }
}

ResourceRecycleFunc VKPipeline::recycle()
{
    const sp<VKDevice> device = _renderer->device();

    VkPipelineLayout layout = _layout;
    VkDescriptorSetLayout descriptorSetLayout = _descriptor_set_layout;
    VkPipeline pipeline = _pipeline;
    _pipeline = VK_NULL_HANDLE;

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
    const PipelineDescriptor& pipelineDescriptor = drawingContext._bindings->pipelineDescriptor();
    _rebind_needed = shouldRebind(graphicsContext.tick(), pipelineDescriptor) || _rebind_needed;

    if(_rebind_needed)
        setupDescriptorSet(graphicsContext, drawingContext._bindings->pipelineDescriptor());

    bindUBOShapshots(graphicsContext, drawingContext._buffer_object->_ubos);
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
    bindUBOShapshots(graphicsContext, computeContext._buffer_object->_ubos);
    buildComputeCommandBuffer(graphicsContext, computeContext);
}

void VKPipeline::setupVertexDescriptions(const PipelineInput& input, VKPipeline::VertexLayout& vertexLayout)
{
    uint32_t location = 0;
    for(const auto& [divsor, stream] : input.streamLayouts())
    {
        vertexLayout.bindingDescriptions.push_back(vks::initializers::vertexInputBindingDescription(
                                                   divsor,
                                                   stream.stride(),
                                                   divsor == 0 ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE));

        for(const Attribute& j : stream.attributes().values())
        {
            const uint32_t slen = std::min<uint32_t>(4, j.length());
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

void VKPipeline::setupDescriptorSetLayout(const PipelineDescriptor& pipelineDescriptor)
{
    const sp<VKDevice>& device = _renderer->device();

    const PipelineInput& pipelineInput = pipelineDescriptor.input();
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    uint32_t binding = 0;
    for(const sp<PipelineInput::UBO>& i : pipelineInput.ubos())
    {
        binding = std::max(binding, i->binding());
        if(shouldStageNeedBinded(i->_stages))
        {
            const VkShaderStageFlags stages = i->_stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, Enum::SHADER_STAGE_BIT_COUNT);
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stages, i->binding()));
        }
    }
    for(const PipelineInput::SSBO& i : pipelineInput.ssbos())
    {
        binding = std::max(binding, i._binding);
        if(shouldStageNeedBinded(i._stages))
        {
            const VkShaderStageFlags stages = i._stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, Enum::SHADER_STAGE_BIT_COUNT);
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stages, i._binding));
        }
    }

    const uint32_t bindingBase = binding + 1;
    for(const auto& [_binding, _stages] : pipelineDescriptor.layout()->samplers())
        if(shouldStageNeedBinded(_stages))
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, Enum::SHADER_STAGE_BIT_COUNT), bindingBase + _binding));

    for(const auto& [_binding, _stages] : pipelineDescriptor.layout()->images())
        if(shouldStageNeedBinded(_stages))
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, Enum::SHADER_STAGE_BIT_COUNT), bindingBase + _binding));

    const VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
    VKUtil::checkResult(vkCreateDescriptorSetLayout(device->vkLogicalDevice(), &descriptorLayout, nullptr, &_descriptor_set_layout));

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&_descriptor_set_layout, 1);
    VKUtil::checkResult(vkCreatePipelineLayout(device->vkLogicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &_layout));
}

void VKPipeline::setupDescriptorSet(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor)
{
    const sp<VKDevice>& device = _renderer->device();

    const VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(_descriptor_pool->vkDescriptorPool(), &_descriptor_set_layout, 1);
    VKUtil::checkResult(vkResetDescriptorPool(device->vkLogicalDevice(), _descriptor_pool->vkDescriptorPool(), 0));
    VKUtil::checkResult(vkAllocateDescriptorSets(device->vkLogicalDevice(), &allocInfo, &_descriptor_set));

    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    uint32_t binding = 0;

    _ubos.clear();
    for(const sp<PipelineInput::UBO>& i : pipelineDescriptor.input()->ubos())
    {
        binding = std::max(binding, i->binding());
        if(shouldStageNeedBinded(i->_stages))
        {
            sp<VKBuffer> ubo = sp<VKBuffer>::make(_renderer, _recycler, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            ubo->uploadBuffer(graphicsContext, sp<UploaderArray<uint8_t>>::make(std::vector<uint8_t>(i->size(), 0)));
            writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                              _descriptor_set,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                              i->binding(),
                                              &ubo->vkDescriptor()));
            _ubos.push_back(std::move(ubo));
        }
    }

    for(const PipelineInput::SSBO& i : pipelineDescriptor.input()->ssbos())
    {
        binding = std::max(binding, i._binding);
        if(shouldStageNeedBinded(i._stages))
        {
            const sp<VKBuffer> sbo = i._buffer.delegate();
            writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                              _descriptor_set,
                                              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                              i._binding,
                                              &sbo->vkDescriptor()));
        }
    }
    const uint32_t bindingBase = binding + 1;
    _texture_observers.clear();
    for(const auto& [i, bindingSet] : pipelineDescriptor.samplers())
        if(shouldStageNeedBinded(bindingSet._stages))
        {
            CHECK_WARN(i, "Pipeline has unbound sampler");
            if(i)
            {
                const sp<VKTexture> texture = i->delegate();
                _texture_observers.push_back(texture->observer().addBooleanSignal());
                if(texture->vkDescriptor().imageView && texture->vkDescriptor().imageLayout)
                    writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                                  _descriptor_set,
                                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                  bindingBase + bindingSet._binding,
                                                  &texture->vkDescriptor()));
            }
        }
    for(const auto& [i, bindingSet] : pipelineDescriptor.images())
        if(shouldStageNeedBinded(bindingSet._stages))
        {
            CHECK_WARN(i, "Pipeline has unbound image");
            if(i)
            {
                const sp<VKTexture> texture = i->delegate();
                _texture_observers.push_back(texture->observer().addBooleanSignal());
                if(texture->vkDescriptor().imageView && texture->vkDescriptor().imageLayout)
                    writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                                  _descriptor_set,
                                                  VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                  bindingBase + bindingSet._binding,
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
                VKUtil::toPrimitiveTopology(_pipeline_descriptor.mode()),
                0,
                VK_FALSE);

    const PipelineDescriptor::PipelineTraitTable& traits =_pipeline_descriptor.parameters()._traits;
    const VkPipelineRasterizationStateCreateInfo rasterizationState = makeRasterizationState(traits);
    const VkPipelineDepthStencilStateCreateInfo depthStencilState = makeDepthStencilState(traits);
    const VkPipelineColorBlendAttachmentState colorBlendAttachmentState = makeColorBlendAttachmentState(traits);
    const VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT };

    if(_pipeline_descriptor.hasFlag(PipelineDescriptor::FLAG_DYNAMIC_SCISSOR, PipelineDescriptor::FLAG_DYNAMIC_SCISSOR_BITMASK))
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    const VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), static_cast<uint32_t>(dynamicStateEnables.size()), 0);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for(const auto& [k, v] : _stages)
        shaderStages.push_back(VKUtil::createShader(device->vkLogicalDevice(), v, k));

    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.traits().ensure<VKGraphicsContext>();
    VKGraphicsContext::State& state = vkGraphicsContext->currentState();
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(_layout, state.acquireRenderPass(_pipeline_descriptor), 0);

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
    uint32_t colorAttachmentCount = std::max<uint32_t>(_pipeline_descriptor.layout()->colorAttachmentCount(), state.renderPassPhrase()->colorAttachmentCount());
    for(uint32_t i = 0; i < colorAttachmentCount; ++i)
    {
        //TODO: MRT only albedo needs blending for now, what about the others?
        VkPipelineColorBlendAttachmentState cbaState = colorBlendAttachmentState;
        cbaState.blendEnable = cbaState.blendEnable && i == 0;
        blendAttachmentStates.push_back(cbaState);
    }
    CHECK_WARN(!blendAttachmentStates.empty(), "Graphics pipeline has no color attachment");
    VkPipelineColorBlendStateCreateInfo colorBlendState =
            vks::initializers::pipelineColorBlendStateCreateInfo(
                static_cast<uint32_t>(blendAttachmentStates.size()),
                blendAttachmentStates.data());

    VkRect2D vkScissors;
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    viewportState.pScissors = &vkScissors;
    if(const Optional<Rect>& scissor = _pipeline_descriptor.scissor())
        vkScissors = VkRect2D({{static_cast<int32_t>(scissor->left()), static_cast<int32_t>(scissor->top())}, {static_cast<uint32_t>(scissor->width()), static_cast<uint32_t>(scissor->height())}});
    else
        vkScissors = VkRect2D({{0, 0}, {state.renderPassPhrase()->resolution().width, state.renderPassPhrase()->resolution().height}});

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
    const VkPipelineShaderStageCreateInfo stage = VKUtil::createShader(device->vkLogicalDevice(), _stages.begin()->second, _stages.begin()->first);
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(_layout, 0);
    computePipelineCreateInfo.stage = stage;
    vkCreateComputePipelines(device->vkLogicalDevice(), device->vkPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &_pipeline);
    vkDestroyShaderModule(device->vkLogicalDevice(), stage.module, nullptr);
}

void VKPipeline::buildDrawCommandBuffer(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) const
{
    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.traits().ensure<VKGraphicsContext>();
    VKGraphicsContext::State& state = vkGraphicsContext->currentState();
    const VkCommandBuffer commandBuffer = state.ensureCommandBuffer();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _layout, 0, 1, &_descriptor_set, 0, nullptr);

    VkDeviceSize offsets = 0;
    VkBuffer vkVertexBuffer = (VkBuffer)(drawingContext._vertices.id());
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &offsets);
    if(drawingContext._indices)
        vkCmdBindIndexBuffer(commandBuffer, (VkBuffer)(drawingContext._indices.id()), 0, kVKIndexType);

    if(const Optional<Rect>& scissor = drawingContext._scissor)
    {
        CHECK(drawingContext._bindings->pipelineDescriptor()->hasFlag(PipelineDescriptor::FLAG_DYNAMIC_SCISSOR, PipelineDescriptor::FLAG_DYNAMIC_SCISSOR_BITMASK), "Pipeline has no DYNAMIC_SCISSOR flag set");
        const VkRect2D vkScissor{{static_cast<int32_t>(scissor->left()), static_cast<int32_t>(scissor->top())}, {static_cast<uint32_t>(scissor->width()), static_cast<uint32_t>(scissor->height())}};
        vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
    }

    _baked_renderer->draw(graphicsContext, drawingContext, commandBuffer);
}

void VKPipeline::buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    const sp<VKComputeContext>& vkContext = graphicsContext.traits().ensure<VKComputeContext>();
    const VkCommandBuffer commandBuffer = vkContext->buildCommandBuffer(graphicsContext);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _layout, 0, 1, &_descriptor_set, 0, nullptr);
    vkCmdDispatch(commandBuffer, computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));
}

sp<VKDescriptorPool> VKPipeline::makeDescriptorPool() const
{
    std::map<VkDescriptorType, uint32_t> poolSizes;
    if(!_pipeline_descriptor.input()->ubos().empty())
        poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = static_cast<uint32_t>(_pipeline_descriptor.input()->ubos().size());
    if(!_pipeline_descriptor.input()->ssbos().empty())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = static_cast<uint32_t>(_pipeline_descriptor.input()->ssbos().size());
    if(!(_pipeline_descriptor.samplers().empty() && _pipeline_descriptor.images().empty()))
        poolSizes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = static_cast<uint32_t>(_pipeline_descriptor.samplers().size() + _pipeline_descriptor.images().size());
    if(!_pipeline_descriptor.images().empty())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] = static_cast<uint32_t>(_pipeline_descriptor.images().size());
    return sp<VKDescriptorPool>::make(_recycler, _renderer->device(), std::move(poolSizes));
}

void VKPipeline::bindUBOShapshots(GraphicsContext& graphicsContext, const std::vector<RenderLayerSnapshot::UBOSnapshot>& uboSnapshots) const
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

bool VKPipeline::shouldStageNeedBinded(const ShaderStageSet& stages) const
{
    return !_is_compute_pipeline || stages.has(Enum::SHADER_STAGE_BIT_COMPUTE);
}

bool VKPipeline::shouldRebind(int64_t tick, const PipelineDescriptor& pipelineDescriptor) const
{
    bool rebindNeeded = false;
    for(const auto& [i, bindingSet] : pipelineDescriptor.samplers())
        if(i->update(tick))
            rebindNeeded = true;
    for(const auto& [i, bindingSet] : pipelineDescriptor.images())
        if(i->update(tick))
            rebindNeeded = true;
    for(const sp<Boolean>& i : _texture_observers)
        if(i->val())
            rebindNeeded = true;
    return rebindNeeded;
}

}
