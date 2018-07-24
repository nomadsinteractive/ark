#include "renderer/base/gl_model_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

GLModelBuffer::GLModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const size_t growCapacity, uint32_t stride, int32_t texCoordinateOffset)
    : _vertices(resourceLoaderContext->memoryPool(), resourceLoaderContext->objectPool(), stride, growCapacity),
      _tex_coordinate_offset(texCoordinateOffset), _normal_offset(-1), _tangents_offset(-1), _indice_base(0)
{
}

void GLModelBuffer::setPosition(float x, float y, float z)
{
    _vertices.write<V3>(_transform.mapXYZ(V3(x, y, z)) + _translate, 0);
}

void GLModelBuffer::setTexCoordinate(uint16_t u, uint16_t v)
{
    if(_tex_coordinate_offset >= 0)
    {
        const uint16_t uv[2] = {u, v};
        _vertices.write(uv, _tex_coordinate_offset);
    }
}

void GLModelBuffer::setNormal(const V3& normal)
{
    if(_normal_offset > 0)
        _vertices.write(normal, _normal_offset);
}

void GLModelBuffer::setTangents(const V3& tangents)
{
    if(_tangents_offset > 0)
        _vertices.write(tangents, _tangents_offset);
}

void GLModelBuffer::applyVaryings()
{
    _vertices.apply(_varyings._bytes);
}

void GLModelBuffer::nextVertex()
{
    _vertices.next();
    applyVaryings();
}

void GLModelBuffer::writeIndices(const glindex_t* indices, glindex_t count)
{
    for(glindex_t i = 0; i < count; i++)
        _index_buffer.push_back(indices[i] + _indice_base);
}

void GLModelBuffer::nextModel()
{
    _indice_base = static_cast<glindex_t>(_vertices.size() / _vertices.stride());
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

const GLBuffer::Builder& GLModelBuffer::vertices() const
{
    return _vertices;
}

GLBuffer::Builder& GLModelBuffer::vertices()
{
    return _vertices;
}

const GLBuffer::Snapshot& GLModelBuffer::indices() const
{
    return _indices;
}

void GLModelBuffer::setIndices(GLBuffer::Snapshot indices)
{
    _indices = std::move(indices);
}

}
