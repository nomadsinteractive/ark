#include "renderer/base/vertex_stream.h"

#include "graphics/base/v4.h"

#include "renderer/base/model.h"

namespace ark {

VertexStream::VertexStream(const PipelineInput::Attributes& attributes, bool doTransform, uint8_t* ptr, size_t size, size_t stride)
    : VertexStream(attributes, doTransform, sp<WriterMemory>::make(ptr, size, stride))
{
}

VertexStream::VertexStream(const PipelineInput::Attributes& attributes, bool doTransform, sp<VertexStream::Writer> writer)
    : _attributes(attributes), _writer(std::move(writer)), _do_transform(doTransform), _visible(true)
{
}

void VertexStream::writePosition(const V3& position)
{
    _writer->writePosition(_visible ? (_do_transform ? (_transform->transform(position) + _translate) : position) : V3());
}

void VertexStream::writeTexCoordinate(uint16_t u, uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    write(uv, _attributes._offsets, PipelineInput::ATTRIBUTE_NAME_TEX_COORDINATE);
}

void VertexStream::writeBoneInfo(const Mesh::BoneInfo& boneInfo)
{
    write(boneInfo._ids, _attributes._offsets, PipelineInput::ATTRIBUTE_NAME_BONE_IDS);
    write(boneInfo._weights, _attributes._offsets, PipelineInput::ATTRIBUTE_NAME_BONE_WEIGHTS);
}

void VertexStream::setRenderObject(const Renderable::Snapshot& renderObject)
{
    _transform = &renderObject._transform;
    _translate = renderObject._position;
    _varyings = renderObject._varyings;
    _visible = renderObject._visible;
}

void VertexStream::writeNormal(const V3& normal)
{
    write(normal, _attributes._offsets, PipelineInput::ATTRIBUTE_NAME_NORMAL);
}

void VertexStream::writeTangent(const V3& tangent)
{
    write(tangent, _attributes._offsets, PipelineInput::ATTRIBUTE_NAME_TANGENT);
}

void VertexStream::writeBitangent(const V3& bitangent)
{
    write(bitangent, _attributes._offsets, PipelineInput::ATTRIBUTE_NAME_BITANGENT);
}

void VertexStream::next()
{
    _writer->next();
    applyVaryings();
}

void VertexStream::applyVaryings()
{
    _writer->write(_varyings._memory.buf(), static_cast<uint32_t>(_varyings._memory.length()), 0);
}

VertexStream::WriterMemory::WriterMemory(uint8_t* ptr, uint32_t size, uint32_t stride)
    : _ptr(nullptr), _begin(ptr), _end(_begin + size), _stride(stride)
{
}

void VertexStream::WriterMemory::next()
{
    _ptr = _ptr ? _ptr + _stride : _begin;
    DCHECK(_ptr <= _end - _stride, "Writer buffer out of bounds");
}

void VertexStream::WriterMemory::writePosition(const V3& position)
{
    write(&position, sizeof(position), 0);
}

uint32_t VertexStream::WriterMemory::write(const void* ptr, uint32_t size, uint32_t offset)
{
    DCHECK(_ptr, "BufferWriter is uninitialized, call nextVertex() first");
    DCHECK(size + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", size, offset);
    memcpy(_ptr + offset, ptr, size);
    return size;
}

}
