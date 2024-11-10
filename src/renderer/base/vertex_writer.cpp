#include "renderer/base/vertex_writer.h"

#include "graphics/base/v4.h"

#include "renderer/base/model.h"

namespace ark {

VertexWriter::VertexWriter(const PipelineInput::AttributeOffsets& attributes, bool doTransform, uint8_t* ptr, size_t size, size_t stride)
    : VertexWriter(attributes, doTransform, sp<WriterMemory>::make(ptr, size, stride))
{
}

VertexWriter::VertexWriter(const PipelineInput::AttributeOffsets& attributes, bool doTransform, sp<VertexWriter::Writer> writer)
    : _attribute_offsets(attributes), _writer(std::move(writer)), _do_transform(doTransform), _visible(true), _transform_snapshot(nullptr)
{
}

bool VertexWriter::hasAttribute(int32_t name) const
{
    return _attribute_offsets._offsets[name] >= 0;
}

void VertexWriter::writePosition(const V3& position)
{
    DASSERT(!_do_transform || _transform_snapshot);
    _writer->writeObject<V3>(_visible ? (_do_transform ? (_transform->transform(*_transform_snapshot, {position, 1.0f}).toNonHomogeneous() + _translate) : position) : V3());
}

void VertexWriter::writeTexCoordinate(uint16_t u, uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    writeAttribute(uv, Attribute::USAGE_TEX_COORD);
}

void VertexWriter::writeBoneInfo(const Mesh::BoneInfo& boneInfo)
{
    writeAttribute(boneInfo._ids, Attribute::USAGE_BONE_IDS);
    writeAttribute(boneInfo._weights, Attribute::USAGE_BONE_WEIGHTS);
}

void VertexWriter::write(const void* buf, uint32_t size, uint32_t offset)
{
    _writer->write(buf, size, offset);
}

void VertexWriter::setRenderable(const Renderable::Snapshot& renderObject)
{
    _transform = renderObject._transform.get();
    _transform_snapshot = &renderObject._transform_snapshot;
    _translate = renderObject._position;
    _varying_contents = renderObject._varyings_snapshot.getDivided(0)._content;
    _visible = renderObject._state.has(Renderable::RENDERABLE_STATE_VISIBLE);
}

void VertexWriter::writeNormal(const V3& normal)
{
    writeAttribute(normal, Attribute::USAGE_NORMAL);
}

void VertexWriter::writeTangent(const V3& tangent)
{
    writeAttribute(tangent, Attribute::USAGE_TANGENT);
}

void VertexWriter::writeBitangent(const V3& bitangent)
{
    writeAttribute(bitangent, Attribute::USAGE_BITANGENT);
}

void VertexWriter::next()
{
    _writer->nextVertex();
    writeVaryings();
}

void VertexWriter::writeVaryings()
{
    _writer->write(_varying_contents.buf(), static_cast<uint32_t>(_varying_contents.length()), 0);
}

VertexWriter::WriterMemory::WriterMemory(uint8_t* ptr, uint32_t size, uint32_t stride)
    : _ptr(nullptr), _begin(ptr), _end(_begin + size), _stride(stride)
{
}

void VertexWriter::WriterMemory::nextVertex()
{
    _ptr = _ptr ? _ptr + _stride : _begin;
    DCHECK(_ptr <= _end - _stride, "Writer buffer out of bounds");
}

uint32_t VertexWriter::WriterMemory::write(const void* ptr, uint32_t size, uint32_t offset)
{
    DCHECK(_ptr, "BufferWriter is uninitialized, call nextVertex() first");
    DCHECK(size + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", size, offset);
    memcpy(_ptr + offset, ptr, size);
    return size;
}

}
