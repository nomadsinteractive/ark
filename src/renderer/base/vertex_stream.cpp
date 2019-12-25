#include "renderer/base/vertex_stream.h"

#include "graphics/base/v4.h"

namespace ark {

VertexStream::VertexStream(const Buffer::Attributes& attributes, uint8_t* ptr, size_t size, size_t stride, bool doTransform)
    : _attributes(attributes), _ptr(nullptr), _begin(ptr), _end(_begin + size), _stride(stride), _do_transform(doTransform), _visible(true)
{
}

void VertexStream::writeArray(ByteArray& array)
{
    DCHECK(array.length() <= _stride, "Varyings buffer overflow: stride: %d, varyings size: %d", _stride, array.length());
    DCHECK(_ptr, "BufferWriter is uninitialized, call next() first");
    DCHECK(_ptr + _stride <= _end, "Varyings buffer out of bounds");
    memcpy(_ptr, array.buf(), array.length());
}

void VertexStream::writePosition(const V3& position)
{
    write<V3>(_visible ? position : V3(), 0);
}

void VertexStream::writePosition(float x, float y, float z)
{
    const V3 position(x, y, z);
    writePosition(_visible && _do_transform ? (_transform->transform(position) + _translate) : position);
}

void VertexStream::writeTexCoordinate(uint16_t u, uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    write(uv, _attributes._offsets, Buffer::ATTRIBUTE_NAME_TEX_COORDINATE);
}

void VertexStream::writeModelId(int32_t modelId)
{
    write(modelId, _attributes._offsets, Buffer::ATTRIBUTE_NAME_MODEL_ID);
}

void VertexStream::setTranslate(const V3& translate)
{
    _translate = translate;
}

void VertexStream::setRenderObject(const Renderable::Snapshot& renderObject)
{
    _transform = &renderObject._transform;
    _varyings = renderObject._varyings;
    _visible = renderObject._visible;
}

void VertexStream::writeNormal(float x, float y, float z)
{
    writeNormal(V3(x, y, z));
}

void VertexStream::writeNormal(const V3& normal)
{
    write(normal, _attributes._offsets, Buffer::ATTRIBUTE_NAME_NORMAL);
}

void VertexStream::writeTangent(float x, float y, float z)
{
    writeTangent(V3(x, y, z));
}

void VertexStream::writeTangent(const V3& tangent)
{
    write(tangent, _attributes._offsets, Buffer::ATTRIBUTE_NAME_TANGENT);
}

void VertexStream::writeBitangent(float x, float y, float z)
{
    writeBitangent(V3(x, y, z));
}

void VertexStream::writeBitangent(const V3& bitangent)
{
    write(bitangent, _attributes._offsets, Buffer::ATTRIBUTE_NAME_BITANGENT);
}

void VertexStream::next()
{
    _ptr = _ptr ? _ptr + _stride : _begin;
    DCHECK(_ptr <= _end - _stride, "Writer buffer out of bounds");
    applyVaryings();
}

void VertexStream::applyVaryings()
{
    if(_varyings._memory.length())
        writeArray(_varyings._memory);
}

}
