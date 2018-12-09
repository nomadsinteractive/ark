#include "renderer/base/model_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

ModelBuffer::ModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<ShaderBindings>& shaderBindings, size_t instanceCount, uint32_t stride)
    : _shader_bindings(shaderBindings), _vertices(resourceLoaderContext->memoryPool(), resourceLoaderContext->objectPool(), stride, instanceCount),
      _instanced_buffer_builders(shaderBindings->makeInstancedBufferBuilders(resourceLoaderContext->memoryPool(), resourceLoaderContext->objectPool(), instanceCount)),
      _indice_base(0), _is_instanced(shaderBindings->instancedArrays().size() > 0)
{
}

void ModelBuffer::writePosition(const V3& position)
{
    _vertices.write<V3>(_is_instanced ? position : _transform.mapXYZ(position) + _translate, 0);
}

void ModelBuffer::writePosition(float x, float y, float z)
{
    writePosition(V3(x, y, z));
}

void ModelBuffer::writeTexCoordinate(uint16_t u, uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    _vertices.write(uv, _shader_bindings->attributes()._offsets, ShaderBindings::ATTRIBUTE_NAME_TEX_COORDINATE);
}

void ModelBuffer::writeNormal(float x, float y, float z)
{
    writeNormal(V3(x, y, z));
}

void ModelBuffer::writeNormal(const V3& normal)
{
    _vertices.write(normal, _shader_bindings->attributes()._offsets, ShaderBindings::ATTRIBUTE_NAME_NORMAL);
}

void ModelBuffer::writeTangent(float x, float y, float z)
{
    writeTangent(V3(x, y, z));
}

void ModelBuffer::writeTangent(const V3& tangent)
{
    _vertices.write(tangent, _shader_bindings->attributes()._offsets, ShaderBindings::ATTRIBUTE_NAME_TANGENT);
}

void ModelBuffer::writeBitangent(float x, float y, float z)
{
    writeBitangent(V3(x, y, z));
}

void ModelBuffer::writeBitangent(const V3& bitangent)
{
    _vertices.write(bitangent, _shader_bindings->attributes()._offsets, ShaderBindings::ATTRIBUTE_NAME_BITANGENT);
}

void ModelBuffer::applyVaryings()
{
    _vertices.apply(_varyings._bytes);
}

void ModelBuffer::nextVertex()
{
    _vertices.next();
    applyVaryings();
}

void ModelBuffer::nextModel()
{
    _indice_base = static_cast<glindex_t>(_vertices.size() / _vertices.stride());
}

void ModelBuffer::setTranslate(const V3& translate)
{
    _translate = translate;
}

void ModelBuffer::setRenderObject(const RenderObject::Snapshot& renderObject)
{
    _transform = renderObject._transform;
    _varyings = renderObject._varyings;
}

const Transform::Snapshot& ModelBuffer::transform() const
{
    return _transform;
}

const Buffer::Builder& ModelBuffer::vertices() const
{
    return _vertices;
}

Buffer::Builder& ModelBuffer::vertices()
{
    return _vertices;
}

const Buffer::Snapshot& ModelBuffer::indices() const
{
    return _indices;
}

void ModelBuffer::setIndices(Buffer::Snapshot indices)
{
    _indices = std::move(indices);
}

bool ModelBuffer::isInstanced() const
{
    return _is_instanced;
}

Buffer::Builder& ModelBuffer::getInstancedArrayBuilder(uint32_t divisor)
{
    auto iter = _instanced_buffer_builders.find(divisor);
    DCHECK(iter != _instanced_buffer_builders.end(), "No instance buffer builder(%d) found", divisor);
    return iter->second;
}

std::vector<std::pair<uint32_t, Buffer::Snapshot>> ModelBuffer::makeInstancedBufferSnapshots() const
{
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots;
    DCHECK(_instanced_buffer_builders.size() == _shader_bindings->_instanced_arrays.size(), "Instanced buffer size mismatch: %d, %d", _instanced_buffer_builders.size(), _shader_bindings->_instanced_arrays.size());

    for(const std::pair<uint32_t, Buffer>& i : _shader_bindings->_instanced_arrays)
        snapshots.emplace_back(i.first, i.second.snapshot(_instanced_buffer_builders.at(i.first).makeUploader()));

    return snapshots;
}

}
