#include "renderer/base/drawing_buffer.h"

#include "core/inf/array.h"
#include "core/base/memory_pool.h"

#include "graphics/base/v3.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

DrawingBuffer::DrawingBuffer(const sp<ShaderBindings>& shaderBindings, uint32_t stride)
    : _shader_bindings(shaderBindings), _pipeline_bindings(_shader_bindings->pipelineBindings()), _vertices(stride),
      _divided_buffer_builders(_shader_bindings->makeDividedBufferBuilders()),
      _is_instanced(_pipeline_bindings->hasDivisors())
{
}

VertexWriter DrawingBuffer::makeVertexStream(const RenderRequest& renderRequest, size_t length, size_t offset)
{
    size_t size = length * _vertices._stride;
    ByteArray::Borrowed content = renderRequest.allocator().sbrk(size);
    _vertices.addStrip(offset * _vertices._stride, content);
    return VertexWriter(_pipeline_bindings->attributes(), !_is_instanced, content.buf(), size, _vertices._stride);
}

VertexWriter DrawingBuffer::makeDividedVertexStream(const RenderRequest& renderRequest, size_t length, size_t offset, uint32_t divisor)
{
    Buffer::Factory& builder = getInstancedBufferBuilder(divisor);

    size_t size = length * builder._stride;
    ByteArray::Borrowed content = renderRequest.allocator().sbrk(size);
    builder.addStrip(offset * builder._stride, content);
    return VertexWriter(_pipeline_bindings->attributes(), !_is_instanced, content.buf(), size, builder._stride);
}

const sp<ShaderBindings>& DrawingBuffer::shaderBindings() const
{
    return _shader_bindings;
}

const Buffer::Factory& DrawingBuffer::vertices() const
{
    return _vertices;
}

Buffer::Factory& DrawingBuffer::vertices()
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

Buffer::Factory& DrawingBuffer::getInstancedBufferBuilder(uint32_t divisor)
{
    auto iter = _divided_buffer_builders.find(divisor);
    DCHECK(iter != _divided_buffer_builders.end(), "No instance buffer builder(%d) found", divisor);
    return iter->second;
}

std::vector<std::pair<uint32_t, Buffer::Snapshot>> DrawingBuffer::toDividedBufferSnapshots()
{
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots;
    DCHECK(_divided_buffer_builders.size() == _shader_bindings->divisors()->size(), "Instanced buffer size mismatch: %d, %d", _divided_buffer_builders.size(), _shader_bindings->divisors()->size());

    for(const auto& i : *(_shader_bindings->divisors()))
        snapshots.emplace_back(i.first, _divided_buffer_builders.at(i.first).toSnapshot(i.second));

    _divided_buffer_builders.clear();
    return snapshots;
}

}
