#include "renderer/base/gl_elements_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"

#include "graphics/base/v3.h"

namespace ark {

GLElementsBuffer::GLElementsBuffer(const sp<MemoryPool>& memoryPool, const uint32_t growCapacity, uint32_t stride, int32_t texCoordinateOffset)
    : _memory_pool(memoryPool), _grow_capacity(growCapacity), _stride(stride), _tex_coordinate_offset(texCoordinateOffset), _normal_offset(-1), _tangents_offset(-1), _indice_base(0), _size(stride), _use_transform(false)
{
    grow();
}

void GLElementsBuffer::setPosition(float x, float y, float z)
{
    if(_use_transform)
    {
        float mx, my, mz;
        _transform.mapXYZ(x, y, z, mx, my, mz);
        (*reinterpret_cast<V3*>(_array_buffer_ptr)) = V3(mx, my, mz) + _translate;
    }
    else
        (*reinterpret_cast<V3*>(_array_buffer_ptr)) = V3(x, y, z);
}

void GLElementsBuffer::setTexCoordinate(float u, float v)
{
    if(_tex_coordinate_offset >= 0)
    {
        uint16_t* uv = reinterpret_cast<uint16_t*>(_array_buffer_ptr + _tex_coordinate_offset);
        uv[0] = static_cast<uint16_t>(u * 0xffff);
        uv[1] = static_cast<uint16_t>(v * 0xffff);
    }
}

void GLElementsBuffer::setTexCoordinate(uint16_t u, uint16_t v)
{
    if(_tex_coordinate_offset >= 0)
    {
        uint16_t* uv = reinterpret_cast<uint16_t*>(_array_buffer_ptr + _tex_coordinate_offset);
        uv[0] = u;
        uv[1] = v;
    }
}

void GLElementsBuffer::setNormal(const V3& normal)
{
    if(_normal_offset > 0)
        (*reinterpret_cast<V3*>(_array_buffer_ptr + _normal_offset)) = normal;
}

void GLElementsBuffer::setTangents(const V3& tangents)
{
    if(_tangents_offset > 0)
        (*reinterpret_cast<V3*>(_array_buffer_ptr + _tangents_offset)) = tangents;
}

void GLElementsBuffer::nextVertex()
{
    if(_array_buffer_ptr == _array_buffer_boundary)
        grow();

    _array_buffer_ptr += _stride;
    _size += _stride;

    DCHECK(_array_buffer_ptr <= _array_buffer_boundary, "Array buffer out of bounds");
}

void GLElementsBuffer::writeIndices(glindex_t* indices, glindex_t count)
{
    for(glindex_t i = 0; i < count; i++)
        _indices.push_back(indices[i] + _indice_base);
}

void GLElementsBuffer::updateIndicesBase()
{
    _indice_base = _size / _stride;
}

void GLElementsBuffer::setTranslate(const V3& translate)
{
    _translate = translate;
}

void GLElementsBuffer::setTransform(const Transform::Snapshot& transform)
{
    _transform = transform;
    _use_transform = true;
}

GLBuffer::Snapshot GLElementsBuffer::getArrayBufferSnapshot(const GLBuffer& arrayBuffer) const
{
    if(_array_buffers.size() == 1)
        return arrayBuffer.snapshot(_array_buffers[0]);
    return arrayBuffer.snapshot();
}

void GLElementsBuffer::grow()
{
    if(_array_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = _memory_pool->allocate(_grow_capacity * _stride);
    _array_buffers.push_back(bytes);
    _array_buffer_ptr = bytes->buf();
    _array_buffer_boundary = _array_buffer_ptr + bytes->length();
}

}
