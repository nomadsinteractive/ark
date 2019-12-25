#include "renderer/base/drawing_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/vertex_stream.h"

namespace ark {

DrawingBuffer::DrawingBuffer(const RenderRequest& renderRequest, const sp<ShaderBindings>& shaderBindings, uint32_t stride)
    : _shader_bindings(shaderBindings), _pipeline_bindings(_shader_bindings->pipelineBindings()), _vertices(renderRequest, _pipeline_bindings->attributes(), stride),
      _divided_buffer_builders(shaderBindings->makeDividedBufferBuilders(renderRequest, _pipeline_bindings->attributes())),
      _is_instanced(_pipeline_bindings->hasDivisors())
{
}

VertexStream DrawingBuffer::makeVertexStream(const RenderRequest& renderRequest, size_t length, size_t offset)
{
    size_t size = length * _vertices._stride;
    ByteArray::Borrowed content = renderRequest.allocator().sbrk(size);
    _vertices.addBlock(offset * _vertices._stride, content);
    return VertexStream(_pipeline_bindings->attributes(), content.buf(), size, _vertices._stride, !_is_instanced);
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
