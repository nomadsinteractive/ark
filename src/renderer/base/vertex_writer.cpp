#include "renderer/base/vertex_writer.h"

#include "core/impl/writable/writable_memory.h"

#include "graphics/base/v4.h"

#include "renderer/base/model.h"

namespace ark {

class VertexWriter::WriterImpl final {
public:
    WriterImpl(sp<Writable> delegate, const uint32_t size, const uint32_t stride)
        : _delegate(std::move(delegate)), _offset(0), _size(size), _stride(stride), _initialized(false)
    {
    }

    void nextVertex()
    {
        if(_initialized)
            _offset += _stride;
        else
            _initialized = true;
        DCHECK(_offset <= _size - _stride, "Writer buffer out of bounds");
    }

    void write(const void* ptr, const uint32_t size, const uint32_t offset) const
    {
        DCHECK(_initialized, "Writer is uninitialized, call nextVertex() first");
        DCHECK(size + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", size, offset);
        _delegate->write(ptr, size, _offset + offset);
    }

private:
    sp<Writable> _delegate;
    uint32_t _offset;
    uint32_t _size;
    uint32_t _stride;
    bool _initialized;
};

VertexWriter::VertexWriter(const PipelineLayout::VertexDescriptor& attributes, const bool doTransform, const uint32_t size, const uint32_t stride, uint8_t* ptr)
    : VertexWriter(attributes, doTransform, size, stride, sp<Writable>::make<WritableMemory>(ptr))
{
}

VertexWriter::VertexWriter(const PipelineLayout::VertexDescriptor& attributes, const bool doTransform, const uint32_t size, const uint32_t stride, sp<Writable> writer)
    : _attribute_offsets(attributes), _delegate(sp<WriterImpl>::make(std::move(writer), size, stride)), _stride(stride), _do_transform(doTransform), _visible(true), _transform_snapshot(nullptr)
{
}

uint32_t VertexWriter::stride() const
{
    return _stride;
}

bool VertexWriter::hasAttribute(const int32_t name) const
{
    return _attribute_offsets._offsets[name] >= 0;
}

void VertexWriter::writePosition(const V3 position)
{
    DASSERT(!_do_transform || _transform_snapshot);
    const V3 obj = _visible ? (_do_transform ? (_transform->transform(*_transform_snapshot, {position, 1.0f}).toNonHomogeneous() + _translate) : position) : V3();
    _delegate->write(&obj, sizeof(V3), 0);
}

void VertexWriter::writeTexCoordinate(const uint16_t u, const uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    writeAttribute(uv, Attribute::USAGE_TEX_COORD);
}

void VertexWriter::writeBoneInfo(const Mesh::BoneInfo& boneInfo)
{
    writeAttribute(boneInfo._ids, Attribute::USAGE_BONE_IDS);
    writeAttribute(boneInfo._weights, Attribute::USAGE_BONE_WEIGHTS);
}

void VertexWriter::write(const void* buf, const uint32_t size, const uint32_t offset)
{
    _delegate->write(buf, size, offset);
}

void VertexWriter::setRenderable(const Renderable::Snapshot& renderObject)
{
    _transform = renderObject._transform.get();
    _transform_snapshot = &renderObject._transform_snapshot;
    _translate = renderObject._position;
    _varying_contents = renderObject._varyings_snapshot.getDivided(0)._content;
    _visible = renderObject._state.contains(Renderable::RENDERABLE_STATE_VISIBLE);
}

void VertexWriter::writeNormal(const V3 normal)
{
    writeAttribute(normal, Attribute::USAGE_NORMAL);
}

void VertexWriter::writeTangent(const V3 tangent)
{
    writeAttribute(tangent, Attribute::USAGE_TANGENT);
}

void VertexWriter::writeBitangent(const V3 bitangent)
{
    writeAttribute(bitangent, Attribute::USAGE_BITANGENT);
}

void VertexWriter::next()
{
    _delegate->nextVertex();
    writeVaryings();
}

void VertexWriter::writeVaryings()
{
    _delegate->write(_varying_contents.buf(), static_cast<uint32_t>(_varying_contents.length()), 0);
}

}
