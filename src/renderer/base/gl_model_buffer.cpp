#include "renderer/base/gl_model_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/gl_shader_bindings.h"

namespace ark {

GLModelBuffer::GLModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<GLShaderBindings>& shaderBindings, size_t instanceCount, uint32_t stride, int32_t texCoordinateOffset)
    : _shader_bindings(shaderBindings), _vertices(resourceLoaderContext->memoryPool(), resourceLoaderContext->objectPool(), stride, instanceCount),
      _instanced_buffer_builders(shaderBindings->makeInstancedBufferBuilders(resourceLoaderContext->memoryPool(), resourceLoaderContext->objectPool(), instanceCount)),
      _tex_coordinate_offset(texCoordinateOffset), _normal_offset(-1), _tangents_offset(-1), _indice_base(0), _is_instanced(false)
{
}

void GLModelBuffer::setPosition(float x, float y, float z)
{
    _vertices.write<V3>(_is_instanced ? V3(x, y, z) : _transform.mapXYZ(V3(x, y, z)) + _translate, 0);
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

bool GLModelBuffer::isInstanced() const
{
    return _is_instanced;
}

void GLModelBuffer::setIsInstanced(bool isInstanced)
{
    _is_instanced = isInstanced;
}

GLBuffer::Builder& GLModelBuffer::getInstancedArrayBuilder(uint32_t divisor)
{
    auto iter = _instanced_buffer_builders.find(divisor);
    DCHECK(iter != _instanced_buffer_builders.end(), "No instance buffer builder(%d) found", divisor);
    return iter->second;
}

std::vector<std::pair<uint32_t, GLBuffer::Snapshot>> GLModelBuffer::makeInstancedBufferSnapshots() const
{
    std::vector<std::pair<uint32_t, GLBuffer::Snapshot>> snapshots;
    DCHECK(_instanced_buffer_builders.size() == _shader_bindings->_instanced_arrays.size(), "Instanced buffer size mismatch: %d, %d", _instanced_buffer_builders.size(), _shader_bindings->_instanced_arrays.size());

    for(const std::pair<uint32_t, GLBuffer>& i : _shader_bindings->_instanced_arrays)
        snapshots.emplace_back(i.first, i.second.snapshot(_instanced_buffer_builders.at(i.first).makeUploader()));

    return snapshots;
}

}
