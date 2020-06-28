#include "renderer/base/pipeline_input.h"

#include "core/base/allocator.h"
#include "core/base/memory_pool.h"
#include "core/base/string.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"

namespace ark {

PipelineInput::PipelineInput()
    : _sampler_count(0)
{
    _streams.insert(std::make_pair(0, Stream()));
}

void PipelineInput::initialize(const PipelineBuildingContext& buildingContext)
{
    std::map<int32_t, sp<PipelineInput::UBO>> ubos;

    for(const sp<Uniform>& i : buildingContext._uniforms.values())
    {
        DCHECK(i->binding() >= 0, "Uniform %s has unspecified binding", i->name().c_str());
        sp<PipelineInput::UBO>& ubo = ubos[i->binding()];
        if(!ubo)
            ubo = sp<PipelineInput::UBO>::make(i->binding());
        ubo->addUniform(i);
    }

    for(auto& i : ubos)
    {
        i.second->initialize();
        _ubos.push_back(std::move(i.second));
    }

    _sampler_count = buildingContext.hasStage(Shader::SHADER_STAGE_FRAGMENT) ?
                     buildingContext.getStage(Shader::SHADER_STAGE_FRAGMENT)->_declaration_samplers.vars().size() : 0;
}

const std::vector<sp<PipelineInput::UBO>>& PipelineInput::ubos() const
{
    return _ubos;
}

const std::map<uint32_t, PipelineInput::Stream>& PipelineInput::streams() const
{
    return _streams;
}

std::map<uint32_t, PipelineInput::Stream>& PipelineInput::streams()
{
    return _streams;
}

size_t PipelineInput::samplerCount() const
{
    return _sampler_count;
}

void PipelineInput::addAttribute(String name, Attribute attribute)
{
    _streams[attribute.divisor()].addAttribute(std::move(name), std::move(attribute));
}

const PipelineInput::Stream& PipelineInput::getStream(uint32_t divisor) const
{
    const auto iter = _streams.find(divisor);
    DCHECK(iter != _streams.end(), "PipelineInput has no stream(%d)", divisor);
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

sp<Uniform> PipelineInput::getUniform(const String& name) const
{
    for(const sp<UBO>& i : _ubos)
    {
        const auto iter = i->uniforms().find(name);
        if(iter != i->uniforms().end())
            return iter->second;
    }
    return nullptr;
}

PipelineInput::Stream::Stream()
    : _stride(0)
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
    return _attributes.has(name) ? static_cast<int32_t>(_attributes.at(name).offset()) : -1;
}

void PipelineInput::Stream::align()
{
    uint32_t mod = _stride % sizeof(float);
    if(mod != 0)
        _stride += (sizeof(float) - mod);
}

PipelineInput::UBO::UBO(uint32_t binding)
    : _binding(binding)
{
}

void PipelineInput::UBO::doSnapshot(uint64_t timestamp, bool force) const
{
    uint8_t* buf = _buffer->buf();
    uint8_t* dirtyFlags = _dirty_flags->buf();
    const std::vector<sp<Uniform>>& uniforms = _uniforms.values();
    for(size_t i = 0; i < uniforms.size(); ++i)
    {
        const Uniform& uniform = uniforms.at(i);
        const sp<Flatable>& flatable = uniform.flatable();
        bool dirty = flatable && flatable->update(timestamp);
        dirtyFlags[i] = static_cast<uint8_t>(force || dirty);
        if(dirtyFlags[i] && flatable)
            flatable->flat(buf);
        buf += uniform.size();
    }
}

RenderLayer::UBOSnapshot PipelineInput::UBO::snapshot(const RenderRequest& renderRequest) const
{
    doSnapshot(renderRequest.timestamp(), false);
    RenderLayer::UBOSnapshot ubo {
        renderRequest.allocator().sbrk(_dirty_flags->size()),
        renderRequest.allocator().sbrk(_buffer->size())
    };
    memcpy(ubo._dirty_flags.buf(), _dirty_flags->buf(), _dirty_flags->size());
    memcpy(ubo._buffer.buf(), _buffer->buf(), _buffer->size());
    return ubo;
}

uint32_t PipelineInput::UBO::binding() const
{
    return _binding;
}

size_t PipelineInput::UBO::size() const
{
    return _buffer->length();
}

const Table<String, sp<Uniform> >& PipelineInput::UBO::uniforms() const
{
    return _uniforms;
}

const std::vector<std::pair<uintptr_t, size_t>>& PipelineInput::UBO::slots() const
{
    return _slots;
}

void PipelineInput::UBO::addStage(Shader::Stage stage)
{
    _stages.insert(stage);
}

const std::set<Shader::Stage>& PipelineInput::UBO::stages() const
{
    return _stages;
}

void PipelineInput::UBO::initialize()
{
    size_t offset = 0;
    for(const auto& i : _uniforms.values())
    {
        size_t s = i->size();
        _slots.push_back(std::make_pair(offset, s));
        offset += s;
        DCHECK(offset % 4 == 0, "Uniform aligment error, offset: %d", offset);
    }
    _buffer = sp<ByteArray::Allocated>::make(offset);
    memset(_buffer->buf(), 0, _buffer->size());

    _dirty_flags = (sp<ByteArray::Allocated>::make(_uniforms.size()));

    doSnapshot(0, true);
}

void PipelineInput::UBO::addUniform(const sp<Uniform>& uniform)
{
    _uniforms.push_back(uniform->name(), uniform);
}

}
