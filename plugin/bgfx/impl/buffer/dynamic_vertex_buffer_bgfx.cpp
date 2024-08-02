#include "bgfx/impl/buffer/dynamic_vertex_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

DynamicVertexBufferBgfx::DynamicVertexBufferBgfx()
    : BufferBase(Buffer::TYPE_VERTEX, Buffer::USAGE_DYNAMIC)
{
}

uint64_t DynamicVertexBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc DynamicVertexBufferBgfx::recycle()
{
    return _handle.recycle();
}

void DynamicVertexBufferBgfx::setupLayout(const PipelineDescriptor& pipelineDescriptor)
{
    setupVertexBufferLayout(_vertex_buffer_layout, pipelineDescriptor);
}

void DynamicVertexBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    if(!_handle)
    {
        ASSERT(_vertex_buffer_layout.m_stride);
        _handle.reset(::bgfx::createDynamicVertexBuffer(_size, _vertex_buffer_layout));
    }

    DASSERT(_size <= _data.size());
    ::bgfx::update(_handle, 0, ::bgfx::makeRef(_data.data(), _size));
}

void DynamicVertexBufferBgfx::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    _data = UploaderType::toBytes(input);
    _size = _data.size();
    upload(graphicsContext);
}

void DynamicVertexBufferBgfx::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    FATAL("Unimplemented");
}

void DynamicVertexBufferBgfx::bind()
{
    ::bgfx::setVertexBuffer(0, _handle);
}

}
