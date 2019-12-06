#include "renderer/base/drawing_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

DrawingBuffer::DrawingBuffer(const sp<ShaderBindings>& shaderBindings, size_t instanceCount, uint32_t stride)
    : _shader_bindings(shaderBindings), _pipeline_bindings(_shader_bindings->pipelineBindings()), _vertices(stride, instanceCount),
      _divided_buffer_builders(shaderBindings->makeDividedBufferBuilders(instanceCount)),
      _indice_base(0), _is_instanced(_pipeline_bindings->hasDivisors()), _transform(nullptr)
{
}

void DrawingBuffer::writePosition(const V3& position)
{
    _vertices.write<V3>(position, 0);
}

void DrawingBuffer::writePosition(float x, float y, float z)
{
    const V3 position(x, y, z);
    _vertices.write<V3>(_is_instanced ? position : (_transform->transform(position) + _translate), 0);
}

void DrawingBuffer::writeTexCoordinate(uint16_t u, uint16_t v)
{
    const uint16_t uv[2] = {u, v};
    _vertices.write(uv, _pipeline_bindings->attributes()._offsets, PipelineBindings::ATTRIBUTE_NAME_TEX_COORDINATE);
}

void DrawingBuffer::writeModelId(int32_t modelId)
{
    _vertices.write(modelId, _pipeline_bindings->attributes()._offsets, PipelineBindings::ATTRIBUTE_NAME_MODEL_ID);
}

void DrawingBuffer::writeNormal(float x, float y, float z)
{
    writeNormal(V3(x, y, z));
}

void DrawingBuffer::writeNormal(const V3& normal)
{
    _vertices.write(normal, _pipeline_bindings->attributes()._offsets, PipelineBindings::ATTRIBUTE_NAME_NORMAL);
}

void DrawingBuffer::writeTangent(float x, float y, float z)
{
    writeTangent(V3(x, y, z));
}

void DrawingBuffer::writeTangent(const V3& tangent)
{
    _vertices.write(tangent, _pipeline_bindings->attributes()._offsets, PipelineBindings::ATTRIBUTE_NAME_TANGENT);
}

void DrawingBuffer::writeBitangent(float x, float y, float z)
{
    writeBitangent(V3(x, y, z));
}

void DrawingBuffer::writeBitangent(const V3& bitangent)
{
    _vertices.write(bitangent, _pipeline_bindings->attributes()._offsets, PipelineBindings::ATTRIBUTE_NAME_BITANGENT);
}

void DrawingBuffer::applyVaryings()
{
    if(_varyings._memory.length())
        _vertices.writeArray(_varyings._memory);
}

void DrawingBuffer::nextVertex()
{
    _vertices.next();
    applyVaryings();
}

void DrawingBuffer::nextModel()
{
    _indice_base = static_cast<element_index_t>(_vertices.length());
}

void DrawingBuffer::setTranslate(const V3& translate)
{
    _translate = translate;
}

void DrawingBuffer::setRenderObject(const RenderObject::Snapshot& renderObject)
{
    _transform = &renderObject._transform;
    _varyings = renderObject._varyings;
}

const sp<ShaderBindings>& DrawingBuffer::shaderBindings() const
{
    return _shader_bindings;
}

const Buffer::Builder& DrawingBuffer::vertices() const
{
    return _vertices;
}

Buffer::Builder& DrawingBuffer::vertices()
{
    return _vertices;
}

const Buffer::Snapshot& DrawingBuffer::indices() const
{
    return _indices;
}

void DrawingBuffer::setIndices(Buffer::Snapshot indices)
{
    _indices = std::move(indices);
}

bool DrawingBuffer::isInstanced() const
{
    return _is_instanced;
}

Buffer::Builder& DrawingBuffer::getInstancedArrayBuilder(uint32_t divisor)
{
    auto iter = _divided_buffer_builders.find(divisor);
    DCHECK(iter != _divided_buffer_builders.end(), "No instance buffer builder(%d) found", divisor);
    return iter->second;
}

std::vector<std::pair<uint32_t, Buffer::Snapshot>> DrawingBuffer::makeDividedBufferSnapshots() const
{
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots;
    DCHECK(_divided_buffer_builders.size() == _shader_bindings->divisors()->size(), "Instanced buffer size mismatch: %d, %d", _divided_buffer_builders.size(), _shader_bindings->divisors()->size());

    for(const auto& i : *(_shader_bindings->divisors()))
        snapshots.emplace_back(i.first, _divided_buffer_builders.at(i.first).toSnapshot(i.second));

    return snapshots;
}

}
