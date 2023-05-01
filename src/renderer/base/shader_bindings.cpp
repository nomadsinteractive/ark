#include "renderer/base/shader_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"

namespace ark {

ShaderBindings::ShaderBindings(Buffer vertices, sp<PipelineFactory> pipelineFactory, sp<PipelineBindings> pipelineBindings, std::map<uint32_t, Buffer> dividedBuffers)
    : _vertices(std::move(vertices)), _pipeline_factory(std::move(pipelineFactory)), _pipeline_bindings(std::move(pipelineBindings)), _snippet(_pipeline_bindings->layout()->snippet()),
      _divided_buffers(sp<std::map<uint32_t, Buffer>>::make(std::move(dividedBuffers))), _attachments(sp<ByType>::make())
{
}

const Buffer& ShaderBindings::vertices() const
{
    return _vertices;
}

Buffer& ShaderBindings::vertices()
{
    return _vertices;
}

const sp<PipelineFactory>& ShaderBindings::pipelineFactory() const
{
    return _pipeline_factory;
}

const sp<PipelineBindings>& ShaderBindings::pipelineBindings() const
{
    return _pipeline_bindings;
}

const sp<Snippet>& ShaderBindings::snippet() const
{
    return _snippet;
}

void ShaderBindings::addSnippet(sp<Snippet> snippet)
{
    DCHECK(!_pipeline, "Draw snippet can only be added before pipeline creation");
    _snippet = sp<SnippetLinkedChain>::make(std::move(_snippet), std::move(snippet));
}

const sp<PipelineLayout>& ShaderBindings::pipelineLayout() const
{
    return _pipeline_bindings->layout();
}

const sp<PipelineInput>& ShaderBindings::pipelineInput() const
{
    return _pipeline_bindings->input();
}

const sp<std::map<uint32_t, Buffer>>& ShaderBindings::divisors() const
{
    return _divided_buffers;
}

const sp<ByType>& ShaderBindings::attachments() const
{
    return _attachments;
}

const sp<Pipeline>& ShaderBindings::getPipeline(GraphicsContext& graphicsContext)
{
    if(!_pipeline)
        _pipeline = _pipeline_bindings->getPipeline(graphicsContext, _pipeline_factory);
    return _pipeline;
}

std::map<uint32_t, Buffer::Factory> ShaderBindings::makeDividedBufferFactories() const
{
    std::map<uint32_t, Buffer::Factory> builders;
    const sp<PipelineInput>& pipelineInput = _pipeline_bindings->input();
    for(const auto& i : *_divided_buffers)
    {
        const PipelineInput::Stream& stream = pipelineInput->getStream(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Factory(stream.stride())));
    }
    return builders;
}

}
