#include "renderer/base/gl_model_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

GLModelBuffer::GLModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const uint32_t growCapacity, uint32_t stride, int32_t texCoordinateOffset)
    : _resource_loader_context(resourceLoaderContext), _grow_capacity(growCapacity), _stride(stride), _tex_coordinate_offset(texCoordinateOffset), _normal_offset(-1), _tangents_offset(-1), _indice_base(0), _size(stride)
{
    NOT_NULL(_grow_capacity);
    grow();
}

void GLModelBuffer::setPosition(float x, float y, float z)
{
    (*reinterpret_cast<V3*>(_array_buffer_ptr)) = _transform.mapXYZ(V3(x, y, z)) + _translate;
}

void GLModelBuffer::setTexCoordinate(uint16_t u, uint16_t v)
{
    if(_tex_coordinate_offset >= 0)
    {
        uint16_t* uv = reinterpret_cast<uint16_t*>(_array_buffer_ptr + _tex_coordinate_offset);
        uv[0] = u;
        uv[1] = v;
    }
}

void GLModelBuffer::setNormal(const V3& normal)
{
    if(_normal_offset > 0)
        (*reinterpret_cast<V3*>(_array_buffer_ptr + _normal_offset)) = normal;
}

void GLModelBuffer::setTangents(const V3& tangents)
{
    if(_tangents_offset > 0)
        (*reinterpret_cast<V3*>(_array_buffer_ptr + _tangents_offset)) = tangents;
}

void GLModelBuffer::applyVaryings()
{
    if(_array_buffer_ptr < _array_buffer_boundary)
    {
        DCHECK(_array_buffer_ptr + _stride <= _array_buffer_boundary, "Varyings buffer out of bounds");
        _varyings.apply(_array_buffer_ptr, _stride);
    }
}

void GLModelBuffer::nextVertex()
{
    if(_array_buffer_ptr == _array_buffer_boundary)
        grow();
    else
        _array_buffer_ptr += _stride;

    _size += _stride;
    DCHECK(_array_buffer_ptr <= _array_buffer_boundary, "Array buffer out of bounds");
    applyVaryings();
}

void GLModelBuffer::writeIndices(glindex_t* indices, glindex_t count)
{
    for(glindex_t i = 0; i < count; i++)
        _indices.push_back(indices[i] + _indice_base);
}

void GLModelBuffer::updateIndicesBase()
{
    _indice_base = _size / _stride;
}

void GLModelBuffer::setTranslate(const V3& translate)
{
    _translate = translate;
}

void GLModelBuffer::setRenderObject(const RenderObject::Snapshot& renderObject)
{
    _transform = renderObject._transform;
    _varyings = renderObject._varyings;
    applyVaryings();
}

const Transform::Snapshot& GLModelBuffer::transform() const
{
    return _transform;
}

GLBuffer::Snapshot GLModelBuffer::getArrayBufferSnapshot(const GLBuffer& arrayBuffer) const
{
    if(_array_buffers.size() == 1)
        return arrayBuffer.snapshot(_resource_loader_context->objectPool()->obtain<GLBuffer::ByteArrayUploader>(_array_buffers[0]));
    return arrayBuffer.snapshot();
}

void GLModelBuffer::grow()
{
    if(_array_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = _resource_loader_context->memoryPool()->allocate(_grow_capacity * _stride);
    _array_buffers.push_back(bytes);
    _array_buffer_ptr = bytes->buf();
    _array_buffer_boundary = _array_buffer_ptr + bytes->length();
    applyVaryings();
}

}
