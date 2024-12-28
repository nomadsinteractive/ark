#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark::vulkan {

class VKPipeline final : public Pipeline {
public:
    VKPipeline(const PipelineDescriptor& bindings, const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, std::map<Enum::ShaderStageBit, String> stages);
    ~VKPipeline() override;

    VkPipeline vkPipeline() const;
    VkPipelineLayout vkPipelineLayout() const;
    const VkDescriptorSet& vkDescriptorSet() const;

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

    class BakedRenderer;

private:
    struct VertexLayout {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    void setupVertexDescriptions(const PipelineInput& input, VertexLayout& vertexLayout);
    void setupDescriptorSetLayout(const PipelineDescriptor& pipelineDescriptor);
    void setupDescriptorSet(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor);

    void setupGraphicsPipeline(GraphicsContext& graphicsContext, const VertexLayout& vertexLayout);
    void setupComputePipeline(GraphicsContext& graphicsContext);

    void buildDrawCommandBuffer(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) const;
    void buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& drawingContext);

    sp<VKDescriptorPool> makeDescriptorPool() const;
    void bindUBOShapshots(GraphicsContext& graphicsContext, const std::vector<RenderLayerSnapshot::UBOSnapshot>& uboSnapshots) const;

    VkPipelineDepthStencilStateCreateInfo makeDepthStencilState() const;
    VkPipelineRasterizationStateCreateInfo makeRasterizationState() const;

    bool shouldStageNeedBinded(const ShaderStageSet& stages) const;
    bool shouldRebind(int64_t tick, const PipelineDescriptor& pipelineDescriptor) const;

private:
    PipelineDescriptor _pipeline_descriptor;

    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
    sp<VKDescriptorPool> _descriptor_pool;
    sp<BakedRenderer> _baked_renderer;

    VkPipelineLayout _layout;
    VkDescriptorSetLayout _descriptor_set_layout;
    VkDescriptorSet _descriptor_set;
    VkPipeline _pipeline;

    std::map<Enum::ShaderStageBit, String> _stages;

    std::vector<sp<VKBuffer>> _ubos;

    std::vector<sp<Boolean>> _texture_observers;

    bool _rebind_needed;
    bool _is_compute_pipeline;
};

}
