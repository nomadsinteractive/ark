#include "renderer/base/shader_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/inf/snippet.h"

namespace ark {

ShaderBindings::ShaderBindings(const sp<PipelineFactory>& pipelineFactory, const sp<PipelineBindings>& pipelineBindings, RenderController& renderController)
    : _pipeline_factory(pipelineFactory), _pipeline_bindings(pipelineBindings), _divisors(makeDivisors(renderController)), _attachments(sp<ByType>::make())
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

const sp<Snippet>& ShaderBindings::snippet() const
{
    return _pipeline_bindings->layout()->snippet();
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
        snippet()->preBind(graphicsContext, _pipeline, *this);
    }
    return _pipeline;
}

std::map<uint32_t, Buffer::Builder> ShaderBindings::makeDividedBufferBuilders() const
{
    std::map<uint32_t, Buffer::Builder> builders;
    const sp<PipelineInput>& pipelineInput = _pipeline_bindings->input();
    for(const auto& i : *_divisors)
    {
        const PipelineInput::Stream& stream = pipelineInput->getStream(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Builder(stream.stride())));
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

}
