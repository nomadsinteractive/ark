#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"

#include "renderer/vulkan/base/vk_pipeline.h"

namespace ark::vulkan {

namespace {

class PipelineComposite : public Pipeline {
public:
    PipelineComposite(sp<Pipeline> pipelineDraw, sp<Pipeline> pipelineCompute)
        : _pipeline_draw(std::move(pipelineDraw)), _pipeline_compute(std::move(pipelineCompute)) {
    }

    uint64_t id() override
    {
        return _pipeline_draw->id();
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        _pipeline_draw->upload(graphicsContext);
        _pipeline_compute->upload(graphicsContext);
    }

    ResourceRecycleFunc recycle() override
    {
        ResourceRecycleFunc func1 = _pipeline_draw->recycle();
        ResourceRecycleFunc func2 = _pipeline_compute->recycle();
        return [func1 = std::move(func1), func2 = std::move(func2)] (GraphicsContext& graphicsContext) {
            func1(graphicsContext);
            func2(graphicsContext);
        };
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        _pipeline_draw->draw(graphicsContext, drawingContext);
    }

    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        _pipeline_draw->bind(graphicsContext, drawingContext);
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        _pipeline_compute->compute(graphicsContext, computeContext);
    }

private:
    sp<Pipeline> _pipeline_draw;
    sp<Pipeline> _pipeline_compute;
};

}

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<Recycler>& recycler, const sp<VKRenderer>& renderFactory)
    : _recycler(recycler), _renderer(renderFactory)
{
}

sp<Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
{
    std::map<Enum::ShaderStageBit, String> shaders = bindings.pipelineLayout()->getPreprocessedShaders(graphicsContext.renderContext());
    if(const auto iter = shaders.find(Enum::SHADER_STAGE_BIT_COMPUTE); iter != shaders.end() && shaders.size() > 1)
    {
        sp<Pipeline> pipelineCompute = sp<Pipeline>::make<VKPipeline>(bindings.pipelineDescriptor(), _recycler, _renderer, std::map<Enum::ShaderStageBit, String>{{iter->first, iter->second}});
        shaders.erase(iter);
        sp<Pipeline> pipelineDraw = sp<Pipeline>::make<VKPipeline>(bindings.pipelineDescriptor(), _recycler, _renderer, std::move(shaders));
        return sp<Pipeline>::make<PipelineComposite>(std::move(pipelineDraw), std::move(pipelineCompute));
    }
    return sp<Pipeline>::make<VKPipeline>(bindings.pipelineDescriptor(), _recycler, _renderer, std::move(shaders));
}

}
