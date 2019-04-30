#include "renderer/base/pipeline_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/pipeline.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/inf/snippet.h"

namespace ark {

PipelineBindings::PipelineBindings(RenderModel::Mode mode, const sp<PipelineLayout>& pipelineLayout)
    : _stub(sp<Stub>::make(mode, pipelineLayout))
{
}

RenderModel::Mode PipelineBindings::mode() const
{
    return _stub->_mode;
}

const sp<PipelineLayout>& PipelineBindings::layout() const
{
    return _stub->_layout;
}

const sp<PipelineInput>& PipelineBindings::input() const
{
    return _stub->_input;
}

const sp<Snippet>& PipelineBindings::snippet() const
{
    return _stub->_layout->snippet();
}

const PipelineBindings::Attributes& PipelineBindings::attributes() const
{
    return _stub->_attributes;
}

const std::vector<sp<Texture>>& PipelineBindings::samplers() const
{
    return _stub->_samplers;
}

void PipelineBindings::bindSampler(const sp<Texture>& texture, uint32_t name)
{
    DCHECK(_stub->_samplers.size() > name, "Illegal sampler binding position: %d, sampler count: %d", name, _stub->_samplers.size());
    _stub->_samplers[name] = texture;
}

bool PipelineBindings::hasDivisors() const
{
    return _stub->_input->streams().size() > 1;
}

PipelineBindings::Flag PipelineBindings::getFlag(PipelineBindings::Flag bitmask) const
{
    return static_cast<Flag>(_stub->_flags & static_cast<uint32_t>(bitmask));
}

bool PipelineBindings::hasFlag(PipelineBindings::Flag flag, PipelineBindings::Flag bitmask) const
{
    return flag == getFlag(bitmask);
}

void PipelineBindings::setFlag(PipelineBindings::Flag flag, PipelineBindings::Flag bitmask) const
{
    _stub->_flags = static_cast<uint32_t>(_stub->_flags & ~static_cast<uint32_t>(bitmask)) | static_cast<uint32_t>(flag);
}

sp<Pipeline> PipelineBindings::getPipeline(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory)
{
    if(_pipeline)
    {
        if(_pipeline->id() == 0)
            _pipeline->upload(graphicsContext, nullptr);
        return _pipeline;
    }

    layout()->preCompile(graphicsContext);
    _pipeline = pipelineFactory->buildPipeline(graphicsContext, *this);
    graphicsContext.renderController()->upload(_pipeline, nullptr, RenderController::US_ON_SURFACE_READY);
    _pipeline->upload(graphicsContext, nullptr);
    return _pipeline;
}

PipelineBindings::Attributes::Attributes(const PipelineInput& input)
{
    _offsets[ATTRIBUTE_NAME_TEX_COORDINATE] = input.getAttributeOffset("TexCoordinate");
    _offsets[ATTRIBUTE_NAME_NORMAL] = input.getAttributeOffset("Normal");
    _offsets[ATTRIBUTE_NAME_TANGENT] = input.getAttributeOffset("Tangent");
    _offsets[ATTRIBUTE_NAME_BITANGENT] = input.getAttributeOffset("Bitangent");
}

PipelineBindings::Stub::Stub(RenderModel::Mode mode, const sp<PipelineLayout>& pipelineLayout)
    : _mode(mode), _layout(pipelineLayout), _input(_layout->input()), _attributes(_input), _flags(FLAG_DEFAULT_VALUE)
{
    _samplers.resize(_input->samplerCount());

    const Table<String, sp<Texture>>& samplers = _layout->samplers();
    DWARN(_samplers.size() >= samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", samplers.size(), _samplers.size());

    for(size_t i = 0; i < samplers.values().size(); ++i)
        if(i < _samplers.size())
            _samplers[i] = samplers.values().at(i);
}

}
