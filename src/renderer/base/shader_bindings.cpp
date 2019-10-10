#include "renderer/base/shader_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/inf/snippet.h"

namespace ark {

ShaderBindings::ShaderBindings(const sp<PipelineFactory>& pipelineFactory, const sp<PipelineBindings>& pipelineBindings, RenderController& renderController)
    : ShaderBindings(pipelineFactory, pipelineBindings, renderController, renderController.makeVertexBuffer(), renderController.makeIndexBuffer(Buffer::USAGE_STATIC))
{
}

ShaderBindings::ShaderBindings(const sp<PipelineFactory>& pipelineFactory, const sp<PipelineBindings>& pipelineBindings, RenderController& renderController, const Buffer& vertexBuffer, const Buffer& indexBuffer)
    : _pipeline_factory(pipelineFactory), _pipeline_bindings(pipelineBindings), _vertex_buffer(vertexBuffer), _index_buffer(indexBuffer), _divisors(makeDivisors(renderController))
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

const Buffer& ShaderBindings::vertexBuffer() const
{
    return _vertex_buffer;
}

const Buffer& ShaderBindings::indexBuffer() const
{
    return _index_buffer;
}

const sp<std::map<uint32_t, Buffer>>& ShaderBindings::divisors() const
{
    return _divisors;
}

const ByType& ShaderBindings::attachments() const
{
    return _attachments;
}

ByType& ShaderBindings::attachments()
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

std::map<uint32_t, Buffer::Builder> ShaderBindings::makeDividedBufferBuilders(const sp<ObjectPool>& objectPool, size_t instanceCount) const
{
    std::map<uint32_t, Buffer::Builder> builders;
    const sp<PipelineInput>& pipelineInput = _pipeline_bindings->input();
    for(const auto& i : *_divisors)
    {
        const PipelineInput::Stream& stream = pipelineInput->getStream(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Builder(objectPool, stream.stride(), instanceCount / i.first)));
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
