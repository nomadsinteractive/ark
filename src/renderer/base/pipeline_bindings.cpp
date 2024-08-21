#include "renderer/base/pipeline_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/pipeline.h"

namespace ark {

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

const Table<String, sp<Texture>>& PipelineBindings::samplers() const
{
    return _pipeline_descriptor->samplers();
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
    _pipeline = _pipeline_factory->buildPipeline(graphicsContext, *this);
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
