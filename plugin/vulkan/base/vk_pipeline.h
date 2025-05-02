#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"

#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKPipeline final : public Pipeline {
public:
    VKPipeline(const PipelineBindings& pipelineBindings, const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, Map<enums::ShaderStageBit, String> stages);
    ~VKPipeline() override;

    VkPipeline vkPipeline() const;
    VkPipelineLayout vkPipelineLayout() const;

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext);
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

    class BakedRenderer;

private:
    void addDescriptorSetLayout(VkDevice device, const Vector<VkDescriptorSetLayoutBinding>& setLayoutBindings);

    void setupDescriptorSetLayout(GraphicsContext& graphicsContext);
    void setupDescriptorSet(GraphicsContext& graphicsContext);

    void setupGraphicsPipeline(GraphicsContext& graphicsContext);
    void setupComputePipeline(GraphicsContext& graphicsContext);

    void buildDrawCommandBuffer(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) const;
    void buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& drawingContext);

    sp<VKDescriptorPool> makeDescriptorPool() const;
    void bindUBOShapshots(GraphicsContext& graphicsContext, const Vector<RenderBufferSnapshot::UBOSnapshot>& uboSnapshots) const;

    bool shouldStageNeedBinding(enums::ShaderStageSet stages) const;
    bool shouldRebind(uint64_t tick) const;

private:
    enums::DrawMode _draw_mode;
    PipelineBindings _pipeline_bindings;

    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
    sp<VKDescriptorPool> _descriptor_pool;
    sp<BakedRenderer> _baked_renderer;

    VkPipelineLayout _layout;
    Vector<VkDescriptorSetLayout> _descriptor_set_layouts;
    Vector<VkDescriptorSet> _descriptor_sets;
    VkPipeline _pipeline;

    Map<enums::ShaderStageBit, String> _stages;

    Vector<sp<VKBuffer>> _ubos;
    Vector<sp<Boolean>> _texture_observers;

    bool _rebind_needed;
    bool _is_compute_pipeline;
};

}
