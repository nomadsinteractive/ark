#include "renderer/base/pipeline_input.h"

#include "core/base/allocator.h"
#include "core/base/string.h"
#include "core/impl/writable/writable_memory.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"

namespace ark {


namespace {

class BindingNames {
public:
    BindingNames(std::vector<String>& names)
        : _names(names) {
    }

    void addBindings(const std::vector<String>& names) {
        for(const String& i : names)
            if(_name_set.find(i) == _name_set.end()) {
                _name_set.insert(i);
                _names.push_back(i);
            }
    }

private:
    std::vector<String>& _names;
    std::set<String> _name_set;
};

}


PipelineInput::AttributeOffsets::AttributeOffsets()
    : _last_attribute(ATTRIBUTE_NAME_TEX_COORDINATE)
{
    std::fill_n(_offsets, ATTRIBUTE_NAME_COUNT, -1);
}

PipelineInput::AttributeOffsets::AttributeOffsets(const PipelineInput& input)
    : AttributeOffsets()
{
    const Stream& stream = input.streams().at(0);
    _offsets[ATTRIBUTE_NAME_TEX_COORDINATE] = stream.getAttributeOffset("TexCoordinate");
    _offsets[ATTRIBUTE_NAME_NORMAL] = stream.getAttributeOffset("Normal");
    _offsets[ATTRIBUTE_NAME_TANGENT] = stream.getAttributeOffset("Tangent");
    _offsets[ATTRIBUTE_NAME_BITANGENT] = stream.getAttributeOffset("Bitangent");
    _offsets[ATTRIBUTE_NAME_BONE_IDS] = stream.getAttributeOffset("BoneIds");
    _offsets[ATTRIBUTE_NAME_BONE_WEIGHTS] = stream.getAttributeOffset("BoneWeights");
    if(input.streams().size() > 1)
    {
        const Stream& stream1 = input.streams().at(1);
        _offsets[ATTRIBUTE_NAME_MODEL_MATRIX] = stream1.getAttributeOffset("Model");
        _offsets[ATTRIBUTE_NAME_NODE_ID] = stream1.getAttributeOffset("NodeId");
        _offsets[ATTRIBUTE_NAME_MATERIAL_ID] = stream1.getAttributeOffset("MaterialId");
    }

    int32_t lastOffset = -1;
    for(int32_t i = ATTRIBUTE_NAME_TEX_COORDINATE; i < ATTRIBUTE_NAME_COUNT; ++i)
    {
        if(lastOffset < _offsets[i])
        {
            _last_attribute = static_cast<AttributeName>(i);
            lastOffset = _offsets[i];
        }
    }
}

size_t PipelineInput::AttributeOffsets::stride() const
{
    if(_offsets[_last_attribute] < 0)
        return 0;

    switch(_last_attribute)
    {
        case ATTRIBUTE_NAME_MODEL_MATRIX:
            return _offsets[ATTRIBUTE_NAME_MODEL_MATRIX] + sizeof(M4);
        case ATTRIBUTE_NAME_NODE_ID:
        case ATTRIBUTE_NAME_MATERIAL_ID:
            return _offsets[_last_attribute] + sizeof(int32_t);
        default:
            return _offsets[_last_attribute];
    }
}

PipelineInput::PipelineInput()
    : _streams{{0, Stream()}}
{
}

void PipelineInput::initialize(const PipelineBuildingContext& buildingContext)
{
    std::map<int32_t, sp<PipelineInput::UBO>> ubos;

    for(const sp<Uniform>& i : buildingContext._uniforms.values())
    {
        CHECK_WARN(i->binding() >= 0, "Uniform \"%s\" has unspecified binding. (Declared but unused variables might be optimized out)", i->name().c_str());
        if(i->binding() >= 0)
        {
            sp<UBO>& ubo = ubos[i->binding()];
            if(!ubo)
                ubo = sp<UBO>::make(i->binding());
            ubo->addUniform(i);
        }
    }

    for(auto& i : ubos)
    {
        i.second->initialize();
        _ubos.push_back(std::move(i.second));
    }

    if(const ShaderPreprocessor* compute = buildingContext.tryGetStage(SHADER_STAGE_COMPUTE))
    {
        _sampler_names = compute->_declaration_samplers.vars().keys();
        _image_names = compute->_declaration_images.vars().keys();
    }
    else
    {
        BindingNames samplerNames(_sampler_names);
        BindingNames imageNames(_image_names);
        if(const ShaderPreprocessor* vertex = buildingContext.tryGetStage(SHADER_STAGE_VERTEX))
        {
            samplerNames.addBindings(vertex->_declaration_samplers.vars().keys());
            imageNames.addBindings(vertex->_declaration_images.vars().keys());
        }
        if(const ShaderPreprocessor* fragment = buildingContext.tryGetStage(SHADER_STAGE_FRAGMENT))
        {
            samplerNames.addBindings(fragment->_declaration_samplers.vars().keys());
            imageNames.addBindings(fragment->_declaration_images.vars().keys());
        }
    }
}

Camera& PipelineInput::camera()
{
    return _camera;
}

const Camera& PipelineInput::camera() const
{
    return _camera;
}

const std::vector<sp<PipelineInput::UBO>>& PipelineInput::ubos() const
{
    return _ubos;
}

std::vector<PipelineInput::SSBO>& PipelineInput::ssbos()
{
    return _ssbos;
}

const std::vector<PipelineInput::SSBO>& PipelineInput::ssbos() const
{
    return _ssbos;
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
    return _sampler_names.size();
}

const std::vector<String>& PipelineInput::samplerNames() const
{
    return _sampler_names;
}

const std::vector<String>& PipelineInput::imageNames() const
{
    return _image_names;
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

Optional<const Attribute&> PipelineInput::getAttribute(const String& name) const
{
    for(const auto& i : _streams)
        if(Optional<const Attribute&> opt = i.second.getAttribute(name))
            return opt;
    return Optional<const Attribute&>();
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

Optional<const Attribute&> PipelineInput::Stream::getAttribute(const String& name) const
{
    return _attributes.has(name) ? Optional<const Attribute&>(_attributes.at(name)) : Optional<const Attribute&>();
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
        const sp<Uploader>& input = uniform.uploader();
        bool dirty = input && input->update(timestamp);
        dirtyFlags[i] = static_cast<uint8_t>(force || dirty);
        if(dirtyFlags[i] && input)
        {
            WritableMemory writable(buf);
            input->upload(writable);
        }
        buf += uniform.size();
    }
}

RenderLayerSnapshot::UBOSnapshot PipelineInput::UBO::snapshot(const RenderRequest& renderRequest) const
{
    doSnapshot(renderRequest.timestamp(), false);
    RenderLayerSnapshot::UBOSnapshot ubo {
        renderRequest.allocator().sbrkSpan(_dirty_flags->size()),
        renderRequest.allocator().sbrkSpan(_buffer->size())
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

void PipelineInput::UBO::addStage(PipelineInput::ShaderStage stage)
{
    _stages.insert(stage);
}

const std::set<PipelineInput::ShaderStage>& PipelineInput::UBO::stages() const
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

PipelineInput::SSBO::SSBO(Buffer buffer, uint32_t binding)
    : _buffer(std::move(buffer)), _binding(binding)
{
}

}
