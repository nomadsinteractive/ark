#include "vulkan/base/vk_pipeline.h"

#include "core/base/observer.h"
#include "core/impl/uploader/uploader_array.h"

#include "renderer/base/buffer.h"
#include "renderer/base/compute_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/pipeline_bindings.h"

#include "vulkan/base/vk_buffer.h"
#include "vulkan/base/vk_command_buffers.h"
#include "vulkan/base/vk_compute_context.h"
#include "vulkan/base/vk_descriptor_pool.h"
#include "vulkan/base/vk_device.h"
#include "vulkan/base/vk_graphics_context.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/base/vk_swap_chain.h"
#include "vulkan/base/vk_texture.h"
#include "vulkan/util/vulkan_tools.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

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
        for(const auto& i : param._instance_buffer_snapshots)
        {
            i.second.upload(graphicsContext);
            DCHECK(i.second.id(), "Invaild Instanced Array Buffer: %d", i.first);
            VkBuffer vkInstanceVertexBuffer = (VkBuffer) (i.second.id());
            vkCmdBindVertexBuffers(commandBuffer, i.first, 1, &vkInstanceVertexBuffer, &offsets);
        }
        vkCmdDrawIndexed(commandBuffer, param._count, drawingContext._draw_count, 0, 0, param._start);
    }
};

struct VKMultiDrawElementsIndirect final : VKPipeline::BakedRenderer {
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override
    {
        VkDeviceSize offsets = 0;
        const DrawingParams::DrawMultiElementsIndirect& param = drawingContext._parameters.drawMultiElementsIndirect();
        for(const auto& [i, j] : param._instance_buffer_snapshots)
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

struct VertexLayout {
    VkPipelineVertexInputStateCreateInfo inputState;
    Vector<VkVertexInputBindingDescription> bindingDescriptions;
    Vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

sp<VKPipeline::BakedRenderer> makeBakedRenderer(const PipelineBindings& pipelineBindings)
{
    switch(pipelineBindings.drawProcedure())
    {
        case enums::DRAW_PROCEDURE_DRAW_ARRAYS:
            return sp<VKPipeline::BakedRenderer>::make<VKDrawArrays>();
        case enums::DRAW_PROCEDURE_DRAW_ELEMENTS:
            return sp<VKPipeline::BakedRenderer>::make<VKDrawElements>();
        case enums::DRAW_PROCEDURE_DRAW_INSTANCED:
            return sp<VKPipeline::BakedRenderer>::make<VKDrawElementsInstanced>();
        case enums::DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT:
            return sp<VKPipeline::BakedRenderer>::make<VKMultiDrawElementsIndirect>();
        default:
            break;
    }
    DFATAL("Not render procedure creator for %d", pipelineBindings.drawProcedure());
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
        const PipelineDescriptor::TraitDepthTest& depthTest = traits.at(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)._depth_test;
        state.depthTestEnable = depthTest._enabled;
        state.depthWriteEnable = depthTest._write_enabled;
        state.depthCompareOp = VKUtil::toCompareOp(depthTest._func);
    }

    if(traits.has(PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST))
    {
        state.stencilTestEnable = true;
        const PipelineDescriptor::TraitStencilTest& stencilTest = traits.at(PipelineDescriptor::TRAIT_TYPE_STENCIL_TEST)._stencil_test;
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
        const PipelineDescriptor::TraitCullFaceTest& cullFaceTest = traits.at(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST)._cull_face_test;
        return vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL, cullFaceTest._enabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE,
                VKUtil::toFrontFace(cullFaceTest._front_face), 0);
    }
    return vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
}

VkPipelineColorBlendAttachmentState makeColorBlendAttachmentState(const PipelineDescriptor::PipelineTraitTable& traits)
{
    VkPipelineColorBlendAttachmentState cbaState = vks::initializers::pipelineColorBlendAttachmentState(0xf, true);
    cbaState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    cbaState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    cbaState.alphaBlendOp = VK_BLEND_OP_ADD;
    cbaState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cbaState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cbaState.colorBlendOp = VK_BLEND_OP_ADD;
    if(traits.has(PipelineDescriptor::TRAIT_TYPE_BLEND))
    {
        const PipelineDescriptor::TraitConfigure& tm = traits.at(PipelineDescriptor::TRAIT_TYPE_BLEND);
        cbaState.blendEnable = tm._blend._enabled;
        if(tm._blend._src_rgb_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.srcColorBlendFactor = toBlendFactor(tm._blend._src_rgb_factor);
        if(tm._blend._dst_rgb_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.dstColorBlendFactor = toBlendFactor(tm._blend._dst_rgb_factor);
        if(tm._blend._src_alpha_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.srcAlphaBlendFactor = toBlendFactor(tm._blend._src_alpha_factor);
        if(tm._blend._dst_alpha_factor != PipelineDescriptor::BLEND_FACTOR_DEFAULT)
            cbaState.dstAlphaBlendFactor = toBlendFactor(tm._blend._dst_alpha_factor);
    }
    return cbaState;
}

VertexLayout setupVertexLayout(const PipelineLayout& pipelineLayout)
{
    uint32_t location = 0;
    VertexLayout vertexLayout;
    for(const auto& [divsor, stream] : pipelineLayout.streamLayouts())
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
    vertexLayout.inputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexLayout.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexLayout.bindingDescriptions.size());
    vertexLayout.inputState.pVertexBindingDescriptions = vertexLayout.bindingDescriptions.data();
    vertexLayout.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.attributeDescriptions.size());
    vertexLayout.inputState.pVertexAttributeDescriptions = vertexLayout.attributeDescriptions.data();

    return vertexLayout;
}

bool testComputePipeline(const Map<enums::ShaderStageBit, String>& stages)
{
    for(const auto& i : stages)
        if(i.first == enums::SHADER_STAGE_BIT_COMPUTE)
        {
            CHECK(stages.size() == 1, "Compute stage is exclusive");
            return true;
        }

    return false;
}

}

VKPipeline::VKPipeline(const PipelineBindings& pipelineBindings, const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, Map<enums::ShaderStageBit, String> stages)
    : _draw_mode(pipelineBindings.drawMode()), _pipeline_bindings(pipelineBindings), _recycler(recycler), _renderer(renderer), _layout(VK_NULL_HANDLE), _pipeline(VK_NULL_HANDLE), _stages(std::move(stages)),
      _rebind_needed(true), _is_compute_pipeline(testComputePipeline(_stages)), _baked_renderer(_is_compute_pipeline ? nullptr : makeBakedRenderer(pipelineBindings))
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

uint64_t VKPipeline::id()
{
    return (uint64_t)(_pipeline);
}

void VKPipeline::upload(GraphicsContext& graphicsContext)
{
    setupDescriptorSetLayout(graphicsContext);
    setupDescriptorSet(graphicsContext);

    if(_is_compute_pipeline)
        setupComputePipeline(graphicsContext);
    else
        setupGraphicsPipeline(graphicsContext);
}

ResourceRecycleFunc VKPipeline::recycle()
{
    const sp<VKDevice> device = _renderer->device();

    VkPipelineLayout layout = _layout;
    VkPipeline pipeline = _pipeline;
    _pipeline = VK_NULL_HANDLE;

    return [device, layout, descriptorSetLayout = std::move(_descriptor_set_layouts), pipeline](GraphicsContext&) {
        if(layout)
            vkDestroyPipelineLayout(device->vkLogicalDevice(), layout, nullptr);
        for(const VkDescriptorSetLayout i : descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device->vkLogicalDevice(), i, nullptr);
        if(pipeline)
            vkDestroyPipeline(device->vkLogicalDevice(), pipeline, nullptr);
    };
}

void VKPipeline::bind(GraphicsContext& graphicsContext, const PipelineContext& pipelineContext)
{
    _rebind_needed = shouldRebind(graphicsContext.tick()) || _rebind_needed;

    if(_rebind_needed)
        setupDescriptorSet(graphicsContext);

    bindUBOShapshots(graphicsContext, pipelineContext._buffer_snapshot->_ubos);
    _rebind_needed = false;
}

void VKPipeline::draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
    DCHECK(!_is_compute_pipeline, "Not a drawing pipeline");

    bind(graphicsContext, drawingContext);
    buildDrawCommandBuffer(graphicsContext, drawingContext);
}

void VKPipeline::compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    DCHECK(_is_compute_pipeline, "Not a compute pipeline");
    bind(graphicsContext, computeContext);
    buildComputeCommandBuffer(graphicsContext, computeContext);
}

void VKPipeline::addDescriptorSetLayout(const VkDevice device, const Vector<VkDescriptorSetLayoutBinding>& setLayoutBindings)
{
    const VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VKUtil::checkResult(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &setLayout));
    _descriptor_set_layouts.push_back(setLayout);
    _descriptor_sets.push_back(VK_NULL_HANDLE);
}

void VKPipeline::setupDescriptorSetLayout(GraphicsContext& graphicsContext)
{
    const sp<VKDevice>& device = _renderer->device();
    const PipelineDescriptor& pipelineDescriptor = _pipeline_bindings.pipelineDescriptor();

    const PipelineLayout& shaderLayout = pipelineDescriptor.layout();
    Vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    Vector<VkDescriptorSetLayoutBinding> setLayoutBindingsUBO;
    for(const sp<PipelineLayout::UBO>& i : shaderLayout.ubos())
        if(shouldStageNeedBinding(i->_stages))
        {
            const VkShaderStageFlags stages = i->_stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, enums::SHADER_STAGE_BIT_COUNT);
            setLayoutBindingsUBO.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stages, i->binding()));
        }

    for(const PipelineLayout::SSBO& i : shaderLayout.ssbos())
        if(shouldStageNeedBinding(i._stages))
        {
            const VkShaderStageFlags stages = i._stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, enums::SHADER_STAGE_BIT_COUNT);
            setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stages, i._binding._location));
        }

    Vector<VkDescriptorSetLayoutBinding> setLayoutBindingsTexture;
    for(const auto& [_stages, _binding] : pipelineDescriptor.layout()->samplers().values())
        if(shouldStageNeedBinding(_stages))
            setLayoutBindingsTexture.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, enums::SHADER_STAGE_BIT_COUNT), _binding._location));

    for(const auto& [_stages, _binding] : pipelineDescriptor.layout()->images().values())
        if(shouldStageNeedBinding(_stages))
            setLayoutBindingsTexture.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _stages.toFlags<VkShaderStageFlagBits>(VKUtil::toStage, enums::SHADER_STAGE_BIT_COUNT), _binding._location));

    addDescriptorSetLayout(device->vkLogicalDevice(), setLayoutBindings);
    addDescriptorSetLayout(device->vkLogicalDevice(), setLayoutBindingsUBO);
    addDescriptorSetLayout(device->vkLogicalDevice(), setLayoutBindingsTexture);

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(_descriptor_set_layouts.data(), _descriptor_set_layouts.size());
    VKUtil::checkResult(vkCreatePipelineLayout(device->vkLogicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &_layout));

    _descriptor_pool = makeDescriptorPool();
    _descriptor_pool->upload(graphicsContext);
}

void VKPipeline::setupDescriptorSet(GraphicsContext& graphicsContext)
{
    const sp<VKDevice>& device = _renderer->device();
    const PipelineDescriptor& pipelineDescriptor = _pipeline_bindings.pipelineDescriptor();

    const VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(_descriptor_pool->vkDescriptorPool(), _descriptor_set_layouts.data(), _descriptor_set_layouts.size());
    VKUtil::checkResult(vkResetDescriptorPool(device->vkLogicalDevice(), _descriptor_pool->vkDescriptorPool(), 0));
    VKUtil::checkResult(vkAllocateDescriptorSets(device->vkLogicalDevice(), &allocInfo, _descriptor_sets.data()));

    Vector<VkWriteDescriptorSet> writeDescriptorSets;

    _ubos.clear();
    for(const sp<PipelineLayout::UBO>& i : pipelineDescriptor.layout()->ubos())
    {
        if(shouldStageNeedBinding(i->_stages))
        {
            sp<VKBuffer> ubo = sp<VKBuffer>::make(_renderer, _recycler, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            UploaderArray<uint8_t> uploader(Vector<uint8_t>(i->size(), 0));
            ubo->uploadBuffer(graphicsContext, uploader);
            writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                              _descriptor_sets.at(1),
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                              i->binding(),
                                              &ubo->vkDescriptor()));
            _ubos.push_back(std::move(ubo));
        }
    }

    _rebind_signals.clear();
    for(const PipelineLayout::SSBO& i : pipelineDescriptor.layout()->ssbos())
    {
        if(shouldStageNeedBinding(i._stages))
        {
            const sp<VKBuffer> sbo = i._buffer.delegate();
            _rebind_signals.push_back(sbo->observer().addBooleanSignal());
            writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                              _descriptor_sets.at(0),
                                              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                              i._binding._location,
                                              &sbo->vkDescriptor()));
        }
    }

    for(const auto& [name, i, bindingSet] : _pipeline_bindings.samplers())
        if(shouldStageNeedBinding(bindingSet._stages))
        {
            CHECK_WARN(i, "Pipeline has unbound sampler \"%s\"", name.c_str());
            if(i)
            {
                const sp<VKTexture> texture = i->delegate();
                _rebind_signals.push_back(texture->observer().addBooleanSignal());
                if(texture->vkDescriptor().imageView)
                {
                    CHECK(texture->vkDescriptor().sampler, "Binding sampler \"%s\" with a texture which declared without usage \"sampler\"", name.c_str());
                    writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                                  _descriptor_sets.at(2),
                                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                  bindingSet._binding._location,
                                                  &texture->vkDescriptor()));
                }
            }
        }

    for(const auto& [name, i, bindingSet] : _pipeline_bindings.images())
        if(shouldStageNeedBinding(bindingSet._stages))
        {
            CHECK_WARN(i, "Pipeline has unbound image \"%s\"", name.c_str());
            if(i)
            {
                const sp<VKTexture> texture = i->delegate();
                _rebind_signals.push_back(texture->observer().addBooleanSignal());
                if(texture->vkDescriptor().imageView && texture->vkDescriptor().imageLayout)
                    writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(
                                                  _descriptor_sets.at(2),
                                                  VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                  bindingSet._binding._location,
                                                  &texture->vkDescriptor()));
            }
        }

    vkUpdateDescriptorSets(device->vkLogicalDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VKPipeline::setupGraphicsPipeline(GraphicsContext& graphicsContext)
{
    const sp<VKDevice>& device = _renderer->device();
    const PipelineDescriptor& pipelineDescriptor = _pipeline_bindings.pipelineDescriptor();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VKUtil::toPrimitiveTopology(_draw_mode), 0, VK_FALSE);

    const PipelineDescriptor::PipelineTraitTable& traits = pipelineDescriptor.configuration()._traits;
    const VkPipelineRasterizationStateCreateInfo rasterizationState = makeRasterizationState(traits);
    const VkPipelineDepthStencilStateCreateInfo depthStencilState = makeDepthStencilState(traits);
    const VkPipelineColorBlendAttachmentState colorBlendAttachmentState = makeColorBlendAttachmentState(traits);
    const VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    Vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT };

    if(pipelineDescriptor.hasTrait(PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST))
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    const VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), static_cast<uint32_t>(dynamicStateEnables.size()), 0);

    Vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for(const auto& [k, v] : _stages)
        shaderStages.push_back(VKUtil::createShader(device->vkLogicalDevice(), v, k));

    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.traits().ensure<VKGraphicsContext>();
    VKGraphicsContext::State& state = vkGraphicsContext->currentState();
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(_layout, state.acquireRenderPass(), 0);

    Vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = state.renderPassPhrase()->makeColorBlendAttachmentStates(colorBlendAttachmentState, pipelineDescriptor.layout()->colorAttachmentCount());
    CHECK_WARN(!blendAttachmentStates.empty(), "Graphics pipeline has no color attachment");
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(static_cast<uint32_t>(blendAttachmentStates.size()), blendAttachmentStates.data());

    VkRect2D vkScissors;
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    viewportState.pScissors = &vkScissors;
    if(const sp<Vec4>& scissor = pipelineDescriptor.scissor())
    {
        const Rect scissorRect(scissor->val());
        vkScissors = VkRect2D({{static_cast<int32_t>(scissorRect.left()), static_cast<int32_t>(scissorRect.top())}, {static_cast<uint32_t>(scissorRect.width()), static_cast<uint32_t>(scissorRect.height())}});
    }
    else
        vkScissors = VkRect2D({{0, 0}, {state.renderPassPhrase()->resolution().width, state.renderPassPhrase()->resolution().height}});

    const VertexLayout vertexLayout = setupVertexLayout(pipelineDescriptor.layout());
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
    const VkCommandBuffer commandBuffer = state.ensureRenderPass();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _layout, 0, _descriptor_sets.size(), _descriptor_sets.data(), 0, nullptr);

    VkDeviceSize offsets = 0;
    VkBuffer vkVertexBuffer = (VkBuffer)(drawingContext._vertices.id());
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &offsets);
    if(drawingContext._indices)
        vkCmdBindIndexBuffer(commandBuffer, (VkBuffer)(drawingContext._indices.id()), 0, kVKIndexType);

    if(const Optional<Rect>& scissor = drawingContext._scissor)
    {
        CHECK(drawingContext._bindings->pipelineDescriptor()->hasTrait(PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST), "Pipeline has no scissor_test trait set");
        const VkRect2D vkScissor{{static_cast<int32_t>(scissor->left()), static_cast<int32_t>(scissor->top())}, {static_cast<uint32_t>(scissor->width()), static_cast<uint32_t>(scissor->height())}};
        vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
    }

    _baked_renderer->draw(graphicsContext, drawingContext, commandBuffer);
}

void VKPipeline::buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& computeContext)
{
    const sp<VKGraphicsContext>& vkGraphicsContext = graphicsContext.traits().ensure<VKGraphicsContext>();
    const VkCommandBuffer commandBuffer = vkGraphicsContext->currentState().commandBuffer();
    // const sp<VKComputeContext>& vkComputeContext = graphicsContext.traits().ensure<VKComputeContext>();
    // const VkCommandBuffer commandBuffer = vkComputeContext->buildCommandBuffer(graphicsContext);

    // vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _layout, 0, _descriptor_sets.size(), _descriptor_sets.data(), 0, nullptr);
    vkCmdDispatch(commandBuffer, computeContext._num_work_groups.at(0), computeContext._num_work_groups.at(1), computeContext._num_work_groups.at(2));
}

sp<VKDescriptorPool> VKPipeline::makeDescriptorPool() const
{
    Map<VkDescriptorType, uint32_t> poolSizes;
    const PipelineLayout& shaderLayout = _pipeline_bindings.pipelineDescriptor()->layout();
    if(!shaderLayout.ubos().empty())
        poolSizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = static_cast<uint32_t>(shaderLayout.ubos().size());
    if(!shaderLayout.ssbos().empty())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = static_cast<uint32_t>(shaderLayout.ssbos().size());
    if(!(_pipeline_bindings.samplers().empty() && _pipeline_bindings.images().empty()))
        poolSizes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = static_cast<uint32_t>(_pipeline_bindings.samplers().size() + _pipeline_bindings.images().size());
    if(!_pipeline_bindings.images().empty())
        poolSizes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] = static_cast<uint32_t>(_pipeline_bindings.images().size());
    return sp<VKDescriptorPool>::make(_recycler, _renderer->device(), std::move(poolSizes), _descriptor_set_layouts.size());
}

void VKPipeline::bindUBOShapshots(GraphicsContext& graphicsContext, const Vector<RenderBufferSnapshot::UBOSnapshot>& uboSnapshots) const
{
    DCHECK(uboSnapshots.size() == _ubos.size(), "UBO Snapshot and UBO Layout mismatch: %d vs %d", uboSnapshots.size(), _ubos.size());

    for(size_t i = 0; i < uboSnapshots.size(); ++i)
    {
        const RenderBufferSnapshot::UBOSnapshot& uboSnapshot = uboSnapshots.at(i);
        if(_rebind_needed || isDirty(uboSnapshot._dirty_flags))
        {
            const sp<VKBuffer>& ubo = _ubos.at(i);
            ubo->reload(graphicsContext, uboSnapshot._buffer);
        }
    }
}

bool VKPipeline::shouldStageNeedBinding(const enums::ShaderStageSet stages) const
{
    return _is_compute_pipeline ? stages.contains(enums::SHADER_STAGE_BIT_COMPUTE) : (stages.contains(enums::SHADER_STAGE_BIT_VERTEX) || stages.contains(enums::SHADER_STAGE_BIT_FRAGMENT));
}

bool VKPipeline::shouldRebind(const uint64_t tick) const
{
    bool rebindNeeded = false;
    for(const auto& [name, i, bindingSet] : _pipeline_bindings.samplers())
        if(i->update(tick))
            rebindNeeded = true;
    for(const auto& [name, i, bindingSet] : _pipeline_bindings.images())
        if(i->update(tick))
            rebindNeeded = true;
    for(const sp<Boolean>& i : _rebind_signals)
        if(i->val())
            rebindNeeded = true;
    return rebindNeeded;
}

}
