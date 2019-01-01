#include "renderer/base/pipeline_input.h"

#include "core/base/string.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/ubo.h"

namespace ark {

PipelineInput::PipelineInput()
    : _sampler_count(0)
{
}

void PipelineInput::initialize(const PipelineBuildingContext& buildingContext)
{
    _ubo = sp<UBO>::make(buildingContext._uniforms.values());
    _sampler_count = buildingContext._fragment._samplers.vars().size();
}

const std::vector<Uniform>& PipelineInput::uniforms() const
{
    return _ubo->uniforms();
}

const sp<UBO>& PipelineInput::ubo() const
{
    return _ubo;
}

const std::map<uint32_t, PipelineInput::Stream>& PipelineInput::streams() const
{
    return _streams;
}

std::map<uint32_t, PipelineInput::Stream>& PipelineInput::streams()
{
    return _streams;
}

uint32_t PipelineInput::samplerCount() const
{
    return _sampler_count;
}

void PipelineInput::addAttribute(String name, Attribute attribute)
{
    if(_streams.find(attribute.divisor()) == _streams.end())
        _streams[attribute.divisor()] = Stream(attribute.divisor());

    _streams[attribute.divisor()].addAttribute(std::move(name), std::move(attribute));
}

std::vector<std::pair<uint32_t, Buffer>> PipelineInput::makeInstancedArrays(RenderController& renderController) const
{
    std::vector<std::pair<uint32_t, Buffer>> instancedArrays;
    for(auto iter : _streams)
    {
        uint32_t divisor = iter.first;
        if(divisor != 0)
            instancedArrays.push_back(std::make_pair(divisor, renderController.makeVertexBuffer()));
    }
    return instancedArrays;
}

const PipelineInput::Stream& PipelineInput::getStream(uint32_t divisor) const
{
    const auto iter = _streams.find(divisor);
    DCHECK(iter != _streams.end(), "GLShaderInput has no stream(%d)", divisor);
    return iter->second;
}

const Attribute& PipelineInput::getAttribute(const String& name, uint32_t divisor) const
{
    return getStream(divisor).getAttribute(name);
}

int32_t PipelineInput::getAttributeOffset(const String& name, uint32_t divisor) const
{
    return getStream(divisor).getAttributeOffset(name);
}

PipelineInput::Stream::Stream()
    : _divisor(0), _stride(0)
{
}

PipelineInput::Stream::Stream(uint32_t divisor)
    : _divisor(divisor), _stride(0)
{
}

uint32_t PipelineInput::Stream::stride() const
{
    return _stride;
}

const Table<String, Attribute>& PipelineInput::Stream::attributes() const
{
    return _attributes;
}

void PipelineInput::Stream::addAttribute(String name, Attribute attribute)
{
    DCHECK(!_attributes.has(name), "Attribute \"%s\" has been added already", name.c_str());
    attribute.setOffset(_stride);
    _stride += attribute.size();
    _attributes.push_back(std::move(name), std::move(attribute));
}

const Attribute& PipelineInput::Stream::getAttribute(const String& name) const
{
    return _attributes.at(name);
}

int32_t PipelineInput::Stream::getAttributeOffset(const String& name) const
{
    return _attributes.has(name) ? _attributes.at(name).offset() : -1;
}

void PipelineInput::Stream::align()
{
    uint32_t mod = _stride % sizeof(float);
    if(mod != 0)
        _stride += (sizeof(float) - mod);
}

}
