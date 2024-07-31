#include "renderer/base/shader_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/pipeline.h"

namespace ark {

ShaderBindings::ShaderBindings(Buffer vertices, sp<PipelineFactory> pipelineFactory, sp<PipelineDescriptor> pipelineDescriptor, std::map<uint32_t, Buffer> dividedBuffers)
    : _vertices(std::move(vertices)), _pipeline_factory(std::move(pipelineFactory)), _pipeline_descriptor(std::move(pipelineDescriptor)), _snippet(_pipeline_descriptor->layout()->snippet()),
      _streams(sp<std::map<uint32_t, Buffer>>::make(std::move(dividedBuffers))), _attachments(sp<Traits>::make())
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

const sp<PipelineDescriptor>& ShaderBindings::pipelineDescriptor() const
{
    return _pipeline_descriptor;
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
    return _pipeline_descriptor->layout();
}

const sp<PipelineInput>& ShaderBindings::pipelineInput() const
{
    return _pipeline_descriptor->input();
}

const sp<std::map<uint32_t, Buffer>>& ShaderBindings::streams() const
{
    return _streams;
}

const sp<Traits>& ShaderBindings::attachments() const
{
    return _attachments;
}

const sp<Pipeline>& ShaderBindings::getPipeline(GraphicsContext& graphicsContext)
{
    if(_pipeline)
    {
        if(_pipeline->id() == 0)
            _pipeline->upload(graphicsContext);
        return _pipeline;
    }

    _pipeline_descriptor->layout()->preCompile(graphicsContext);
    _pipeline = _pipeline_factory->buildPipeline(graphicsContext, _pipeline_descriptor);
    graphicsContext.renderController()->upload(_pipeline, RenderController::US_ON_SURFACE_READY, nullptr, nullptr, RenderController::UPLOAD_PRIORITY_HIGH);
    _pipeline->upload(graphicsContext);
    return _pipeline;
}

std::map<uint32_t, Buffer::Factory> ShaderBindings::makeDividedBufferFactories() const
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
