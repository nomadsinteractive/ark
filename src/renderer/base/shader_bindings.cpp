#include "renderer/base/shader_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"

namespace ark {

ShaderBindings::ShaderBindings(const sp<PipelineFactory>& pipelineFactory, const sp<PipelineBindings>& pipelineBindings, RenderController& renderController)
    : _pipeline_factory(pipelineFactory), _pipeline_bindings(pipelineBindings), _snippet_draw(_pipeline_bindings->layout()->snippet()), _divisors(makeDivisors(renderController)),
      _attachments(sp<ByType>::make())
{
}

const sp<PipelineFactory>& ShaderBindings::pipelineFactory() const
{
    return _pipeline_factory;
}

const sp<PipelineBindings>& ShaderBindings::pipelineBindings() const
{
    return _pipeline_bindings;
}

const sp<SnippetDraw>& ShaderBindings::snippet() const
{
    return _snippet_draw;
}

void ShaderBindings::addSnippetDraw(sp<SnippetDraw> snippet)
{
    DCHECK(!_pipeline, "Draw snippet can only be added before pipeline creation");
    _snippet_draw = sp<SnippetDrawLinkedChain>::make(std::move(_snippet_draw), std::move(snippet));
}

const sp<PipelineLayout>& ShaderBindings::pipelineLayout() const
{
    return _pipeline_bindings->layout();
}

const sp<PipelineInput>& ShaderBindings::pipelineInput() const
{
    return _pipeline_bindings->input();
}

const std::vector<sp<Texture>>& ShaderBindings::samplers() const
{
    return _pipeline_bindings->samplers();
}

const sp<std::map<uint32_t, Buffer>>& ShaderBindings::divisors() const
{
    return _divisors;
}

const sp<ByType>& ShaderBindings::attachments() const
{
    return _attachments;
}

sp<Pipeline> ShaderBindings::getPipeline(GraphicsContext& graphicsContext)
{
    if(!_pipeline)
    {
        _pipeline = _pipeline_bindings->getPipeline(graphicsContext, _pipeline_factory);
        _snippet_draw->preBind(graphicsContext, _pipeline, *this);
    }
    return _pipeline;
}

std::map<uint32_t, Buffer::Factory> ShaderBindings::makeDividedBufferBuilders() const
{
    std::map<uint32_t, Buffer::Factory> builders;
    const sp<PipelineInput>& pipelineInput = _pipeline_bindings->input();
    for(const auto& i : *_divisors)
    {
        const PipelineInput::Stream& stream = pipelineInput->getStream(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Factory(stream.stride())));
    }
    return builders;
}

sp<std::map<uint32_t, Buffer>> ShaderBindings::makeDivisors(RenderController& renderController) const
{
    sp<std::map<uint32_t, Buffer>> divisors = sp<std::map<uint32_t, Buffer>>::make();
    for(const auto& i : pipelineInput()->streams())
    {
        uint32_t divisor = i.first;
        if(divisor != 0)
        {
            DCHECK(divisors->find(divisor) == divisors->end(), "Duplicated stream divisor: %d", divisor);
            divisors->insert(std::make_pair(divisor, renderController.makeVertexBuffer()));
        }
    }
    return divisors;
}

ShaderBindings::SnippetDrawLinkedChain::SnippetDrawLinkedChain(sp<SnippetDraw> delegate, sp<SnippetDraw> next)
    : _delegate(std::move(delegate)), _next(std::move(next))
{
    DASSERT(_delegate && _next);
}

void ShaderBindings::SnippetDrawLinkedChain::preBind(GraphicsContext& graphicsContext, const sp<Pipeline>& pipeline, ShaderBindings& bindings)
{
    _delegate->preBind(graphicsContext, pipeline, bindings);
    _next->preBind(graphicsContext, pipeline, bindings);
}

void ShaderBindings::SnippetDrawLinkedChain::preDraw(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    _delegate->preDraw(graphicsContext, context);
    _next->preDraw(graphicsContext, context);
}

void ShaderBindings::SnippetDrawLinkedChain::postDraw(GraphicsContext& graphicsContext)
{
    _delegate->postDraw(graphicsContext);
    _next->postDraw(graphicsContext);
}

}
