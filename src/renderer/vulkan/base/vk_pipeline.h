#pragma once

#include <vector>

#include "core/types/shared_ptr.h"
#include "core/inf/runnable.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKPipeline : public Pipeline {
public:
    VKPipeline(const PipelineBindings& bindings, const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, std::map<PipelineInput::ShaderStage, String> shaders);
    ~VKPipeline() override;

    VkPipeline vkPipeline() const;
    VkPipelineLayout vkPipelineLayout() const;
    const VkDescriptorSet& vkDescriptorSet() const;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual ResourceRecycleFunc recycle() override;

    virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;
    virtual void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override;

private:
    struct VertexLayout {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    void setupVertexDescriptions(const PipelineInput& input, VertexLayout& vertexLayout);
    void setupDescriptorSetLayout(const PipelineInput& pipelineInput);
    void setupDescriptorSet(GraphicsContext& graphicsContext, const PipelineBindings& bindings);

    void setupGraphicsPipeline(GraphicsContext& graphicsContext, const VertexLayout& vertexLayout);
    void setupComputePipeline(GraphicsContext& graphicsContext);

    void buildDrawCommandBuffer(GraphicsContext& graphicsContext, const DrawingContext& drawingContext);
    void buildComputeCommandBuffer(GraphicsContext& graphicsContext, const ComputeContext& drawingContext);

    bool isDirty(const ByteArray::Borrowed& dirtyFlags) const;

    class BakedRenderer {
    public:
        virtual ~BakedRenderer() = default;

        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) = 0;
    };

    class VKDrawArrays : public BakedRenderer {
    public:
        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override;

    };

    class VKDrawElements : public BakedRenderer {
    public:
        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override;

    };

    class VKDrawElementsInstanced : public BakedRenderer {
    public:
        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override;

    };

    class VKMultiDrawElementsIndirect : public BakedRenderer {
    public:
        virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, VkCommandBuffer commandBuffer) override;

    };

    sp<BakedRenderer> makeBakedRenderer(const PipelineBindings& bindings) const;
    sp<VKDescriptorPool> makeDescriptorPool() const;
    void bindUBOShapshots(GraphicsContext& graphicsContext, const std::vector<RenderLayerSnapshot::UBOSnapshot>& uboSnapshots);

    VkPipelineDepthStencilStateCreateInfo makeDepthStencilState() const;
    VkStencilOpState makeStencilState(const PipelineBindings::TraitStencilTestSeparate& stencil) const;
    VkPipelineRasterizationStateCreateInfo makeRasterizationState() const;

private:
    PipelineBindings _bindings;

    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
    sp<VKDescriptorPool> _descriptor_pool;
    sp<BakedRenderer> _baked_renderer;

    VkPipelineLayout _layout;
    VkDescriptorSetLayout _descriptor_set_layout;
    VkDescriptorSet _descriptor_set;
    VkPipeline _pipeline;

    std::map<PipelineInput::ShaderStage, String> _shaders;

    std::vector<sp<VKBuffer>> _ubos;

    std::vector<sp<Boolean>> _texture_observers;

    bool _rebind_needed;
    bool _is_compute_pipeline;
};

}
}
