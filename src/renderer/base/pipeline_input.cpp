#include "renderer/base/pipeline_input.h"

#include "core/base/allocator.h"
#include "core/base/string.h"
#include "core/impl/writable/writable_memory.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"

namespace ark {

namespace {

uint32_t getAttributeEndOffset(const PipelineInput::AttributeOffsets& attrOffsets, Attribute::Usage usage)
{
    switch(usage)
    {
        case Attribute::USAGE_MODEL_MATRIX:
            return attrOffsets._offsets[usage] + sizeof(M4);
        case Attribute::USAGE_NODE_ID:
        case Attribute::USAGE_MATERIAL_ID:
            return attrOffsets._offsets[usage] + sizeof(int32_t);
        default:
            return std::max(0, attrOffsets._offsets[usage]);
    }
}

}

PipelineInput::AttributeOffsets::AttributeOffsets()
    : _stride(0)
{
    std::fill_n(_offsets, Attribute::USAGE_COUNT, -1);
}

PipelineInput::AttributeOffsets::AttributeOffsets(const PipelineInput& input)
    : AttributeOffsets()
{
    const StreamLayout& stream = input.streamLayouts().at(0);
    _offsets[Attribute::USAGE_POSITION] = 0;
    _offsets[Attribute::USAGE_TEX_COORD] = stream.getAttributeOffset(Attribute::USAGE_TEX_COORD);
    _offsets[Attribute::USAGE_NORMAL] = stream.getAttributeOffset(Attribute::USAGE_NORMAL);
    _offsets[Attribute::USAGE_TANGENT] = stream.getAttributeOffset(Attribute::USAGE_TANGENT);
    _offsets[Attribute::USAGE_BITANGENT] = stream.getAttributeOffset(Attribute::USAGE_BITANGENT);
    _offsets[Attribute::USAGE_BONE_IDS] = stream.getAttributeOffset(Attribute::USAGE_BONE_IDS);
    _offsets[Attribute::USAGE_BONE_WEIGHTS] = stream.getAttributeOffset(Attribute::USAGE_BONE_WEIGHTS);
    if(input.streamLayouts().size() > 1)
    {
        const StreamLayout& stream1 = input.streamLayouts().at(1);
        _offsets[Attribute::USAGE_MODEL_MATRIX] = stream1.getAttributeOffset("Model");
        _offsets[Attribute::USAGE_NODE_ID] = stream1.getAttributeOffset("NodeId");
        _offsets[Attribute::USAGE_MATERIAL_ID] = stream1.getAttributeOffset("MaterialId");
    }

    for(int32_t i = Attribute::USAGE_MODEL_MATRIX; i < Attribute::USAGE_COUNT; ++i)
        _stride = std::max(getAttributeEndOffset(*this, static_cast<Attribute::Usage>(i)), _stride);
}

uint32_t PipelineInput::AttributeOffsets::stride() const
{
    return _stride;
}

PipelineInput::PipelineInput(const sp<Camera>& camera)
    : _camera(camera ? *camera : Camera::createDefaultCamera()), _stream_layouts{{0, StreamLayout()}}
{
}

void PipelineInput::initialize(const PipelineBuildingContext& buildingContext)
{
    for(const auto& [k, v] : buildingContext._ubos)
    {
        v->initialize();
        _ubos.push_back(v);
    }
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

sp<RenderLayerSnapshot::BufferObject> PipelineInput::takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const
{
    std::vector<RenderLayerSnapshot::UBOSnapshot> uboSnapshot;
    for(const sp<UBO>& i : _ubos)
        if(isComputeStage ? i->_stages.has(Enum::SHADER_STAGE_BIT_COMPUTE) : i->_stages != Enum::SHADER_STAGE_BIT_COMPUTE)
            uboSnapshot.push_back(i->snapshot(renderRequest));

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> ssboSnapshot;
    for(const SSBO& i : _ssbos)
        if(isComputeStage ? i._stages.has(Enum::SHADER_STAGE_BIT_COMPUTE) : i._stages != Enum::SHADER_STAGE_BIT_COMPUTE)
            ssboSnapshot.emplace_back(i._binding, i._buffer.snapshot());

    return sp<RenderLayerSnapshot::BufferObject>::make(RenderLayerSnapshot::BufferObject{std::move(uboSnapshot), std::move(ssboSnapshot)});
}

const std::map<uint32_t, PipelineInput::StreamLayout>& PipelineInput::streamLayouts() const
{
    return _stream_layouts;
}

std::map<uint32_t, PipelineInput::StreamLayout>& PipelineInput::streamLayouts()
{
    return _stream_layouts;
}

size_t PipelineInput::samplerCount() const
{
    return _sampler_names.size();
}

const std::vector<String>& PipelineInput::samplerNames() const
{
    return _sampler_names;
}

void PipelineInput::addAttribute(String name, Attribute attribute)
{
    _stream_layouts[attribute.divisor()].addAttribute(std::move(name), std::move(attribute));
}

const PipelineInput::StreamLayout& PipelineInput::getStreamLayout(uint32_t divisor) const
{
    const auto iter = _stream_layouts.find(divisor);
    DCHECK(iter != _stream_layouts.end(), "PipelineInput has no stream(%d)", divisor);
    return iter->second;
}

Optional<const Attribute&> PipelineInput::getAttribute(const String& name) const
{
    for(const auto& i : _stream_layouts)
        if(const Optional<const Attribute&> opt = i.second.getAttribute(name))
            return opt;
    return {};
}

sp<Uniform> PipelineInput::getUniform(const String& name) const
{
    for(const sp<UBO>& i : _ubos)
        if(const auto iter = i->uniforms().find(name); iter != i->uniforms().end())
            return iter->second;
    return nullptr;
}

PipelineInput::StreamLayout::StreamLayout()
    : _stride(0)
{
}

uint32_t PipelineInput::StreamLayout::stride() const
{
    return _stride;
}

const Table<String, Attribute>& PipelineInput::StreamLayout::attributes() const
{
    return _attributes;
}

void PipelineInput::StreamLayout::addAttribute(String name, Attribute attribute)
{
    DCHECK(!_attributes.has(name), "Attribute \"%s\" has been added already", name.c_str());
    attribute.setOffset(_stride);
    _stride += attribute.size();
    _attributes.push_back(std::move(name), std::move(attribute));
}

Optional<const Attribute&> PipelineInput::StreamLayout::getAttribute(Attribute::Usage layoutType) const
{
    DASSERT(layoutType != Attribute::USAGE_CUSTOM);
    for(const Attribute& i : _attributes.values())
        if(i.usage() == layoutType)
            return {i};
    return {};
}

Optional<const Attribute&> PipelineInput::StreamLayout::getAttribute(const String& name) const
{
    return _attributes.has(name) ? Optional<const Attribute&>(_attributes.at(name)) : Optional<const Attribute&>();
}

int32_t PipelineInput::StreamLayout::getAttributeOffset(Attribute::Usage layoutType) const
{
    const Optional<const Attribute&> attr = getAttribute(layoutType);
    return attr ? static_cast<int32_t>(attr->offset()) : -1;
}

int32_t PipelineInput::StreamLayout::getAttributeOffset(const String& name) const
{
    return _attributes.has(name) ? static_cast<int32_t>(_attributes.at(name).offset()) : -1;
}

void PipelineInput::StreamLayout::align(uint32_t alignment)
{
    uint32_t mod = _stride % alignment;
    if(mod != 0)
        _stride += (alignment - mod);
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

const Table<String, sp<Uniform>>& PipelineInput::UBO::uniforms() const
{
    return _uniforms;
}

const std::vector<std::pair<uintptr_t, size_t>>& PipelineInput::UBO::slots() const
{
    return _slots;
}

void PipelineInput::UBO::initialize()
{
    size_t offset = 0;
    for(const auto& i : _uniforms.values())
    {
        size_t s = i->size();
        _slots.emplace_back(offset, s);
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

uint32_t PipelineInput::BindingSet::addStage(Enum::ShaderStageBit stage, uint32_t binding)
{
    _stages.set(stage);
    if(_binding != std::numeric_limits<uint32_t>::max())
        return binding;
    _binding = binding;
    return binding + 1;
}

}
