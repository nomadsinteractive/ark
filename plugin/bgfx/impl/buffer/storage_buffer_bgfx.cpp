#include "storage_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

StorageBufferBgfx::StorageBufferBgfx()
    : BufferBase(Buffer::TYPE_STORAGE, Buffer::USAGE_DYNAMIC)
{
    _vertex_buffer_layout.begin().add(::bgfx::Attrib::Position, 4, ::bgfx::AttribType::Float).end();
}

uint64_t StorageBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc StorageBufferBgfx::recycle()
{
    return _handle.recycle();
}

void StorageBufferBgfx::setupLayout(const PipelineDescriptor& pipelineDescriptor)
{
    setupVertexBufferLayout(_vertex_buffer_layout, pipelineDescriptor);
}

void StorageBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    if(!_handle)
    {
        ASSERT(_vertex_buffer_layout.m_stride);
        _handle.reset(::bgfx::createDynamicVertexBuffer(_size / _vertex_buffer_layout.m_stride, _vertex_buffer_layout));
    }

    DASSERT(_size <= _data.size());
    ::bgfx::update(_handle, 0, ::bgfx::makeRef(_data.data(), _size));
}

void StorageBufferBgfx::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    _data = UploaderType::toBytes(input);
    _size = _data.size();
    upload(graphicsContext);
}

void StorageBufferBgfx::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    WARN("Unimplemented");
}

void StorageBufferBgfx::bind()
{
    ::bgfx::setVertexBuffer(0, _handle);
}

}
