#include "renderer/base/drawing_buffer.h"

#include "core/inf/array.h"

#include "graphics/base/v3.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

DrawingBuffer::DrawingBuffer(sp<PipelineBindings> pipelineBindings, uint32_t stride)
    : _pipeline_bindings(std::move(pipelineBindings)), _pipeline_descriptor(_pipeline_bindings->pipelineDescriptor()), _vertices(stride),
      _divided_buffer_builders(_pipeline_bindings->makeDividedBufferFactories()),
      _is_instanced(_pipeline_descriptor->hasDivisors())
{
}

VertexWriter DrawingBuffer::makeVertexWriter(const RenderRequest& renderRequest, size_t length, size_t offset)
{
    const size_t size = length * _vertices._stride;
    ByteArray::Borrowed content = renderRequest.allocator().sbrkSpan(size);
    _vertices.addStrip(offset * _vertices._stride, content);
    return VertexWriter(_pipeline_descriptor->attributes(), !_is_instanced, content.buf(), size, _vertices._stride);
}

VertexWriter DrawingBuffer::makeDividedVertexWriter(const RenderRequest& renderRequest, size_t length, size_t offset, uint32_t divisor)
{
    Buffer::Factory& builder = getDividedBufferBuilder(divisor);

    size_t size = length * builder._stride;
    ByteArray::Borrowed content = renderRequest.allocator().sbrkSpan(size);
    builder.addStrip(offset * builder._stride, content);
    return VertexWriter(_pipeline_descriptor->attributes(), !_is_instanced, content.buf(), size, builder._stride);
}

const sp<PipelineBindings>& DrawingBuffer::pipelineBindings() const
{
    return _pipeline_bindings;
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

Buffer::Factory& DrawingBuffer::getDividedBufferBuilder(uint32_t divisor)
{
    auto iter = _divided_buffer_builders.find(divisor);
    DCHECK(iter != _divided_buffer_builders.end(), "No instance buffer builder(%d) found", divisor);
    return iter->second;
}

std::vector<std::pair<uint32_t, Buffer::Snapshot>> DrawingBuffer::toDividedBufferSnapshots()
{
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots;
    DCHECK(_divided_buffer_builders.size() == _pipeline_bindings->streams()->size(), "Instanced buffer size mismatch: %d, %d", _divided_buffer_builders.size(), _pipeline_bindings->streams()->size());

    for(const auto& [i, j] : *(_pipeline_bindings->streams()))
        snapshots.emplace_back(i, _divided_buffer_builders.at(i).toSnapshot(j));

    _divided_buffer_builders.clear();
    return snapshots;
}

}
