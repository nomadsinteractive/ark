#include "bgfx/impl/buffer/static_vertex_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

StaticVertexBufferBgfx::StaticVertexBufferBgfx()
    : BufferBase(Buffer::Usage(Buffer::USAGE_BIT_VERTEX))
{
}

uint64_t StaticVertexBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc StaticVertexBufferBgfx::recycle()
{
    return _handle.recycle();
}

void StaticVertexBufferBgfx::setupLayout(const PipelineDescriptor& pipelineDescriptor)
{
    setupVertexBufferLayout(_vertex_buffer_layout, pipelineDescriptor);
}

void StaticVertexBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    if(_handle)
        _handle.destroy();

    DASSERT(_vertex_buffer_layout.m_stride > 0);
    DASSERT(_size <= _data.size());
    _handle.reset(::bgfx::createVertexBuffer(::bgfx::copy(_data.data(), _size), _vertex_buffer_layout));
}

void StaticVertexBufferBgfx::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    CHECK(!_handle.isValid(), "Cannot upload data into a static vertex buffer");
    _data = UploaderType::toBytes(input);
    _size = _data.size();
    shiftTexCoords(_vertex_buffer_layout, _data.data(), _size / _vertex_buffer_layout.m_stride);
    upload(graphicsContext);
}

void StaticVertexBufferBgfx::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    FATAL("Unimplemented");
}

void StaticVertexBufferBgfx::bind()
{
    ::bgfx::setVertexBuffer(0, _handle);
}

void StaticVertexBufferBgfx::bindRange(uint32_t first, uint32_t count)
{
    ::bgfx::setVertexBuffer(0, _handle, first, count);
}

}
