#include "renderer/base/vertex_writer.h"

#include "graphics/base/v4.h"

#include "renderer/base/model.h"

namespace ark {

VertexWriter::VertexWriter(const PipelineInput::AttributeOffsets& attributes, bool doTransform, uint8_t* ptr, size_t size, size_t stride)
    : VertexWriter(attributes, doTransform, sp<WriterMemory>::make(ptr, size, stride))
{
}

VertexWriter::VertexWriter(const PipelineInput::AttributeOffsets& attributes, bool doTransform, sp<VertexWriter::Writer> writer)
    : _attribute_offsets(attributes), _writer(std::move(writer)), _do_transform(doTransform), _visible(true), _transform(nullptr)
{
}

bool VertexWriter::hasAttribute(int32_t name) const
{
    return _attribute_offsets._offsets[name] >= 0;
}

void VertexWriter::writePosition(const V3& position)
{
    DASSERT(!_do_transform || _transform);
    _writer->writePosition(_visible ? (_do_transform ? (_transform->transform(position) + _translate) : position) : V3());
}

void VertexWriter::writeTexCoordinate(uint16_t u, uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    writeAttribute(uv, PipelineInput::ATTRIBUTE_NAME_TEX_COORDINATE);
}

void VertexWriter::writeBoneInfo(const Mesh::BoneInfo& boneInfo)
{
    writeAttribute(boneInfo._ids, PipelineInput::ATTRIBUTE_NAME_BONE_IDS);
    writeAttribute(boneInfo._weights, PipelineInput::ATTRIBUTE_NAME_BONE_WEIGHTS);
}

void VertexWriter::writeNodeId(int32_t nodeId)
{
    writeAttribute(nodeId, PipelineInput::ATTRIBUTE_NAME_NODE_ID);
}

void VertexWriter::write(const void* buf, uint32_t size, uint32_t offset)
{
    _writer->write(buf, size, offset);
}

void VertexWriter::setRenderObject(const Renderable::Snapshot& renderObject)
{
    _transform = &renderObject._transform;
    _translate = renderObject._position;
    _varying_contents = renderObject._varyings.getDivided(0)._content;
    _visible = renderObject._state.hasState(Renderable::RENDERABLE_STATE_VISIBLE);
}

void VertexWriter::writeNormal(const V3& normal)
{
    writeAttribute(normal, PipelineInput::ATTRIBUTE_NAME_NORMAL);
}

void VertexWriter::writeTangent(const V3& tangent)
{
    writeAttribute(tangent, PipelineInput::ATTRIBUTE_NAME_TANGENT);
}

void VertexWriter::writeBitangent(const V3& bitangent)
{
    writeAttribute(bitangent, PipelineInput::ATTRIBUTE_NAME_BITANGENT);
}

void VertexWriter::next()
{
    _writer->next();
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

void VertexWriter::WriterMemory::next()
{
    _ptr = _ptr ? _ptr + _stride : _begin;
    DCHECK(_ptr <= _end - _stride, "Writer buffer out of bounds");
}

void VertexWriter::WriterMemory::writePosition(const V3& position)
{
    write(&position, sizeof(position), 0);
}

uint32_t VertexWriter::WriterMemory::write(const void* ptr, uint32_t size, uint32_t offset)
{
    DCHECK(_ptr, "BufferWriter is uninitialized, call nextVertex() first");
    DCHECK(size + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", size, offset);
    memcpy(_ptr + offset, ptr, size);
    return size;
}

}
