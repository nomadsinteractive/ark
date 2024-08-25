#include "renderer/base/pipeline_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/pipeline.h"

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

PipelineBindings::PipelineBindings(Buffer vertices, sp<PipelineFactory> pipelineFactory, sp<PipelineDescriptor> pipelineDescriptor, std::map<uint32_t, Buffer> streams)
    : _vertices(std::move(vertices)), _pipeline_factory(std::move(pipelineFactory)), _pipeline_descriptor(std::move(pipelineDescriptor)), _snippet(_pipeline_descriptor->layout()->snippet()),
      _streams(sp<std::map<uint32_t, Buffer>>::make(std::move(streams))), _attachments(sp<Traits>::make())
{
    if(_vertices)
        _vertices.delegate()->setupLayout(_pipeline_descriptor);
}

const Buffer& PipelineBindings::vertices() const
{
    return _vertices;
}

Buffer& PipelineBindings::vertices()
{
    return _vertices;
}

const sp<PipelineFactory>& PipelineBindings::pipelineFactory() const
{
    return _pipeline_factory;
}

const sp<PipelineDescriptor>& PipelineBindings::pipelineDescriptor() const
{
    return _pipeline_descriptor;
}

const sp<Snippet>& PipelineBindings::snippet() const
{
    return _snippet;
}

void PipelineBindings::addSnippet(sp<Snippet> snippet)
{
    DCHECK(!_pipeline, "Draw snippet can only be added before pipeline creation");
    _snippet = sp<SnippetLinkedChain>::make(std::move(_snippet), std::move(snippet));
}

const sp<PipelineLayout>& PipelineBindings::pipelineLayout() const
{
    return _pipeline_descriptor->layout();
}

const sp<PipelineInput>& PipelineBindings::pipelineInput() const
{
    return _pipeline_descriptor->input();
}

const sp<std::map<uint32_t, Buffer>>& PipelineBindings::streams() const
{
    return _streams;
}

const sp<Traits>& PipelineBindings::attachments() const
{
    return _attachments;
}

const sp<Pipeline>& PipelineBindings::getPipeline(GraphicsContext& graphicsContext)
{
    if(_pipeline)
    {
        if(_pipeline->id() == 0)
            _pipeline->upload(graphicsContext);
        return _pipeline;
    }

    _pipeline_descriptor->layout()->preCompile(graphicsContext);

    std::map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> stages = _pipeline_descriptor->layout()->getPreprocessedStages(graphicsContext.renderContext());
    std::map<Enum::ShaderStageBit, String> sources;
    for(auto& [k, v] : stages)
        sources.emplace(k, std::move(v._source));

    if(const auto iter = sources.find(Enum::SHADER_STAGE_BIT_COMPUTE); iter != sources.end() && sources.size() > 1)
    {
        sp<Pipeline> pipelineCompute = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor, std::map<Enum::ShaderStageBit, String>{{iter->first, iter->second}});
        sources.erase(iter);
        sp<Pipeline> pipelineDraw = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor, std::move(sources));
        _pipeline = sp<Pipeline>::make<PipelineComposite>(std::move(pipelineDraw), std::move(pipelineCompute));
    }
    else
        _pipeline = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor, std::move(sources));
    _pipeline->upload(graphicsContext);
    return _pipeline;
}

std::map<uint32_t, Buffer::Factory> PipelineBindings::makeDividedBufferFactories() const
{
    std::map<uint32_t, Buffer::Factory> builders;
    const sp<PipelineInput>& pipelineInput = _pipeline_descriptor->input();
    for(const auto& i : *_streams)
    {
        const PipelineInput::StreamLayout& stream = pipelineInput->getStreamLayout(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Factory(stream.stride())));
    }
    return builders;
}

}
