#include "renderer/base/pipeline_layout.h"

#include <ranges>

#include "core/base/allocator.h"
#include "core/base/string.h"
#include "core/util/uploader_type.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"

namespace ark {

namespace {

uint32_t getAttributeSize(const Attribute::Usage usage)
{
    switch(usage)
    {
        case Attribute::USAGE_NODE_ID:
        case Attribute::USAGE_MATERIAL_ID:
        case Attribute::USAGE_TEX_COORD:
            return sizeof(int32_t);
        case Attribute::USAGE_POSITION:
            return sizeof(V3);
        case Attribute::USAGE_COLOR:
        case Attribute::USAGE_NORMAL:
        case Attribute::USAGE_TANGENT:
        case Attribute::USAGE_BITANGENT:
        case Attribute::USAGE_BONE_IDS:
            return sizeof(V4);
        case Attribute::USAGE_BONE_WEIGHTS:
        case Attribute::USAGE_MODEL_MATRIX:
            return sizeof(M4);
        default:
            return 0;
    }
}

uint32_t getAttributeEndOffset(const PipelineLayout::VertexDescriptor& attrOffsets, const Attribute::Usage usage)
{
    if(attrOffsets._offsets[usage] < 0)
        return 0;

    return attrOffsets._offsets[usage] + getAttributeSize(usage);
}

const PipelineLayout::StreamLayout& findStreamLayout(const Vector<std::pair<uint32_t, PipelineLayout::StreamLayout>>& streamLayouts, const uint32_t divisor)
{
    for(const auto& [k, v] : streamLayouts)
        if(k == divisor)
            return v;
    FATAL("PipelineInput has no stream(%d)", divisor);
    return streamLayouts.back().second;
}

PipelineLayout::StreamLayout& ensureStreamLayout(Vector<std::pair<uint32_t, PipelineLayout::StreamLayout>>& streamLayouts, const uint32_t divisor)
{
    for(auto& [k, v] : streamLayouts)
        if(k == divisor)
            return v;
    streamLayouts.emplace_back(divisor, PipelineLayout::StreamLayout());
    return streamLayouts.back().second;
}

}

PipelineLayout::VertexDescriptor::VertexDescriptor()
    : _strides{}
{
    std::fill_n(_offsets, Attribute::USAGE_COUNT, -1);
}

PipelineLayout::VertexDescriptor::VertexDescriptor(const PipelineLayout& pipelineLayout)
    : VertexDescriptor()
{
    initialize(pipelineLayout);
}

void PipelineLayout::VertexDescriptor::initialize(const PipelineLayout& pipelineLayout)
{
    const StreamLayout& stream = pipelineLayout.getStreamLayout(0);
    _offsets[Attribute::USAGE_POSITION] = 0;
    _offsets[Attribute::USAGE_TEX_COORD] = stream.getAttributeOffset(Attribute::USAGE_TEX_COORD);
    _offsets[Attribute::USAGE_NORMAL] = stream.getAttributeOffset(Attribute::USAGE_NORMAL);
    _offsets[Attribute::USAGE_TANGENT] = stream.getAttributeOffset(Attribute::USAGE_TANGENT);
    _offsets[Attribute::USAGE_BITANGENT] = stream.getAttributeOffset(Attribute::USAGE_BITANGENT);
    _offsets[Attribute::USAGE_BONE_IDS] = stream.getAttributeOffset(Attribute::USAGE_BONE_IDS);
    _offsets[Attribute::USAGE_BONE_WEIGHTS] = stream.getAttributeOffset(Attribute::USAGE_BONE_WEIGHTS);
    if(pipelineLayout.streamLayouts().size() > 1)
    {
        const StreamLayout& stream1 = pipelineLayout.getStreamLayout(1);
        _offsets[Attribute::USAGE_MODEL_MATRIX] = stream1.getAttributeOffset("Model");
        _offsets[Attribute::USAGE_NODE_ID] = stream1.getAttributeOffset("NodeId");
        _offsets[Attribute::USAGE_MATERIAL_ID] = stream1.getAttributeOffset("MaterialId");
    }

    for(int32_t i = Attribute::USAGE_POSITION; i <= Attribute::USAGE_BONE_WEIGHTS; ++i)
        _strides[0] = std::max(getAttributeEndOffset(*this, static_cast<Attribute::Usage>(i)), _strides[0]);

    for(int32_t i = Attribute::USAGE_MODEL_MATRIX; i <= Attribute::USAGE_MATERIAL_ID; ++i)
        _strides[1] = std::max(getAttributeEndOffset(*this, static_cast<Attribute::Usage>(i)), _strides[1]);
}

PipelineLayout::PipelineLayout()
    : _stream_layouts{{0, StreamLayout()}}, _color_attachment_count(0)
{
}

void PipelineLayout::initialize(const PipelineBuildingContext& buildingContext)
{
    if(const ShaderPreprocessor* fragment = buildingContext.tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT))
        _color_attachment_count = fragment->_main_block->outArgumentCount() + (fragment->_main_block->hasReturnValue() ? 1 : 0);

    for(const auto& v : std::views::values(buildingContext._ubos))
    {
        v->initialize();
        _ubos.push_back(v);
    }

    _vertex_descriptor.initialize(*this);
}

const Vector<sp<PipelineLayout::UBO>>& PipelineLayout::ubos() const
{
    return _ubos;
}

const Vector<PipelineLayout::SSBO>& PipelineLayout::ssbos() const
{
    return _ssbos;
}

sp<RenderBufferSnapshot> PipelineLayout::takeBufferSnapshot(const RenderRequest& renderRequest, const bool isComputeStage) const
{
    Vector<RenderBufferSnapshot::UBOSnapshot> uboSnapshot;
    for(const UBO& i : _ubos)
        if(isComputeStage ? i._stages.has(enums::SHADER_STAGE_BIT_COMPUTE) : (i._stages.has(enums::SHADER_STAGE_BIT_VERTEX) || i._stages.has(enums::SHADER_STAGE_BIT_FRAGMENT)))
            uboSnapshot.push_back(i.snapshot(renderRequest));

    Vector<std::pair<uint32_t, Buffer::Snapshot>> ssboSnapshot;
    for(const SSBO& i : _ssbos)
        if(isComputeStage ? i._stages.has(enums::SHADER_STAGE_BIT_COMPUTE) : (i._stages.has(enums::SHADER_STAGE_BIT_VERTEX) || i._stages.has(enums::SHADER_STAGE_BIT_FRAGMENT)))
            ssboSnapshot.emplace_back(i._binding._location, i._buffer.snapshot());

    return sp<RenderBufferSnapshot>::make(RenderBufferSnapshot{std::move(uboSnapshot), std::move(ssboSnapshot)});
}

const Vector<std::pair<uint32_t, PipelineLayout::StreamLayout>>& PipelineLayout::streamLayouts() const
{
    return _stream_layouts;
}

void PipelineLayout::setStreamLayoutAlignment(const uint32_t alignment)
{
    for(auto &[k, v] : _stream_layouts)
        v.align(k == 0 ? 4 : alignment);
}

const PipelineLayout::VertexDescriptor& PipelineLayout::vertexDescriptor() const
{
    return _vertex_descriptor;
}

const Table<String, PipelineLayout::DescriptorSet>& PipelineLayout::samplers() const
{
    return _samplers;
}

const Table<String, PipelineLayout::DescriptorSet>& PipelineLayout::images() const
{
    return _images;
}

uint32_t PipelineLayout::colorAttachmentCount() const
{
    return _color_attachment_count;
}

void PipelineLayout::addAttribute(String name, Attribute attribute)
{
    ensureStreamLayout(_stream_layouts, attribute.divisor()).addAttribute(std::move(name), std::move(attribute));
}

const PipelineLayout::StreamLayout& PipelineLayout::getStreamLayout(const uint32_t divisor) const
{
    return findStreamLayout(_stream_layouts, divisor);
}

Optional<const Attribute&> PipelineLayout::getAttribute(const String& name) const
{
    for(const auto& v : std::views::values(_stream_layouts))
        if(const Optional<const Attribute&> opt = v.getAttribute(name))
            return opt;
    return {};
}

sp<Uniform> PipelineLayout::getUniform(const String& name) const
{
    for(const UBO& i : _ubos)
        if(const auto iter = i.uniforms().find(name); iter != i.uniforms().end())
            return iter->second;
    return nullptr;
}

PipelineLayout::StreamLayout::StreamLayout()
    : _stride(0)
{
}

uint32_t PipelineLayout::StreamLayout::stride() const
{
    return _stride;
}

const Table<String, Attribute>& PipelineLayout::StreamLayout::attributes() const
{
    return _attributes;
}

void PipelineLayout::StreamLayout::addAttribute(String name, Attribute attribute)
{
    DCHECK(!_attributes.has(name), "Attribute \"%s\" has been added already", name.c_str());
    attribute.setOffset(_stride);
    _stride += attribute.size();
    _attributes.push_back(std::move(name), std::move(attribute));
}

Optional<const Attribute&> PipelineLayout::StreamLayout::getAttribute(Attribute::Usage layoutType) const
{
    DASSERT(layoutType != Attribute::USAGE_CUSTOM);
    for(const Attribute& i : _attributes.values())
        if(i.usage() == layoutType)
            return {i};
    return {};
}

Optional<const Attribute&> PipelineLayout::StreamLayout::getAttribute(const String& name) const
{
    return _attributes.has(name) ? Optional<const Attribute&>(_attributes.at(name)) : Optional<const Attribute&>();
}

int32_t PipelineLayout::StreamLayout::getAttributeOffset(const Attribute::Usage layoutType) const
{
    const Optional<const Attribute&> attr = getAttribute(layoutType);
    return attr ? static_cast<int32_t>(attr->offset()) : -1;
}

int32_t PipelineLayout::StreamLayout::getAttributeOffset(const String& name) const
{
    return _attributes.has(name) ? static_cast<int32_t>(_attributes.at(name).offset()) : -1;
}

void PipelineLayout::StreamLayout::align(const uint32_t alignment)
{
    if(const uint32_t mod = _stride % alignment)
        _stride += (alignment - mod);
}

PipelineLayout::UBO::UBO(const uint32_t binding)
    : _binding(binding)
{
}

void PipelineLayout::UBO::doSnapshot(const uint64_t timestamp, const bool force) const
{
    uint8_t* buf = _buffer->buf();
    uint8_t* dirtyFlags = _dirty_flags->buf();
    const Vector<sp<Uniform>>& uniforms = _uniforms.values();
    for(size_t i = 0; i < uniforms.size(); ++i)
    {
        const Uniform& uniform = uniforms.at(i);
        const sp<Uploader>& uploader = uniform.uploader();
        const bool dirty = uploader && uploader->update(timestamp);
        dirtyFlags[i] = static_cast<uint8_t>(force || dirty);
        if(dirtyFlags[i] && uploader)
            UploaderType::writeTo(uploader, buf);
        buf += uniform.size();
    }
}

RenderBufferSnapshot::UBOSnapshot PipelineLayout::UBO::snapshot(const RenderRequest& renderRequest) const
{
    doSnapshot(renderRequest.timestamp(), false);
    RenderBufferSnapshot::UBOSnapshot ubo = {
        renderRequest.allocator().sbrkSpan(_dirty_flags->size()),
        renderRequest.allocator().sbrkSpan(_buffer->size())
    };
    memcpy(ubo._dirty_flags.buf(), _dirty_flags->buf(), _dirty_flags->size());
    memcpy(ubo._buffer.buf(), _buffer->buf(), _buffer->size());
    return ubo;
}

uint32_t PipelineLayout::UBO::binding() const
{
    return _binding;
}

size_t PipelineLayout::UBO::size() const
{
    return _buffer->length();
}

const Table<String, sp<Uniform>>& PipelineLayout::UBO::uniforms() const
{
    return _uniforms;
}

const Vector<std::pair<uintptr_t, size_t>>& PipelineLayout::UBO::slots() const
{
    return _slots;
}

void PipelineLayout::UBO::initialize()
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

void PipelineLayout::UBO::addUniform(const sp<Uniform>& uniform)
{
    _uniforms.push_back(uniform->name(), uniform);
}

PipelineLayout::SSBO::SSBO(Buffer buffer, const Binding binding)
    : _buffer(std::move(buffer)), _binding(binding)
{
}

uint32_t PipelineLayout::DescriptorSet::addStage(enums::ShaderStageBit stage, uint32_t binding)
{
    _stages.set(stage);
    if(_binding._location != -1)
        return binding;
    _binding = {static_cast<int32_t>(binding)};
    return binding + 1;
}

}
