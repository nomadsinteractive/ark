#include "renderer/base/pipeline_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

namespace {

class PipelineComposite final : public Pipeline {
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

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        _pipeline_compute->compute(graphicsContext, computeContext);
    }

    sp<Pipeline> _pipeline_draw;
    sp<Pipeline> _pipeline_compute;
};

}

PipelineBindings::PipelineBindings(Buffer vertices, sp<PipelineFactory> pipelineFactory, sp<PipelineDescriptor> pipelineDescriptor, Map<uint32_t, Buffer> streams)
    : _vertices(std::move(vertices)), _pipeline_factory(std::move(pipelineFactory)), _pipeline_descriptor(std::move(pipelineDescriptor)),
      _streams(sp<Map<uint32_t, Buffer>>::make(std::move(streams))), _attachments(sp<Traits>::make())
{
}

const Buffer& PipelineBindings::vertices() const
{
    return _vertices;
}

Buffer& PipelineBindings::vertices()
{
    return _vertices;
}

const sp<PipelineDescriptor>& PipelineBindings::pipelineDescriptor() const
{
    return _pipeline_descriptor;
}

const sp<Snippet>& PipelineBindings::snippet() const
{
    return _pipeline_descriptor->layout()->snippet();
}

const sp<ShaderLayout>& PipelineBindings::shaderLayout() const
{
    return _pipeline_descriptor->shaderLayout();
}

const sp<Map<uint32_t, Buffer>>& PipelineBindings::streams() const
{
    return _streams;
}

const sp<Traits>& PipelineBindings::attachments() const
{
    return _attachments;
}

const sp<Pipeline>& PipelineBindings::ensurePipeline(GraphicsContext& graphicsContext)
{
    if(_pipeline)
    {
        if(_pipeline->id() == 0)
            _pipeline->upload(graphicsContext);
    }
    else
        doEnsurePipeline(graphicsContext);

    return _pipeline;
}

const sp<Pipeline>& PipelineBindings::ensureRenderPipeline(GraphicsContext& graphicsContext)
{
    const sp<Pipeline>& pipeline = ensurePipeline(graphicsContext);
    if(pipeline.isInstance<PipelineComposite>())
        return pipeline.cast<PipelineComposite>()->_pipeline_draw;
    return pipeline;
}

Map<uint32_t, Buffer::Factory> PipelineBindings::makeDividedBufferFactories() const
{
    Map<uint32_t, Buffer::Factory> builders;
    const sp<ShaderLayout>& shaderLayout = _pipeline_descriptor->shaderLayout();
    for(const auto& i : *_streams)
    {
        const ShaderLayout::StreamLayout& stream = shaderLayout->getStreamLayout(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Factory(stream.stride())));
    }
    return builders;
}

void PipelineBindings::doEnsurePipeline(GraphicsContext& graphicsContext)
{
    _pipeline_descriptor->layout()->preCompile(graphicsContext);

    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> stages = _pipeline_descriptor->layout()->getPreprocessedStages(graphicsContext.renderContext());
    ASSERT(!stages.empty());

    Map<Enum::ShaderStageBit, String> sources;
    for(auto& [k, v] : stages)
        sources.emplace(k, std::move(v._source));

    if(const auto iter = sources.find(Enum::SHADER_STAGE_BIT_COMPUTE); iter != sources.end() && sources.size() > 1)
    {
        sp<Pipeline> computePipeline = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor, Map<Enum::ShaderStageBit, String>{{iter->first, iter->second}});
        sources.erase(iter);

        sp<Pipeline> renderPipeline = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor, std::move(sources));
        _pipeline = sp<Pipeline>::make<PipelineComposite>(std::move(renderPipeline), std::move(computePipeline));
    }
    else
        _pipeline = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor, std::move(sources));
    _pipeline->upload(graphicsContext);
}

}
