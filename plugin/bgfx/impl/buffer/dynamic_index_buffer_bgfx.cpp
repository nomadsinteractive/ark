#include "bgfx/impl/buffer/dynamic_index_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

DynamicIndexBufferBgfx::DynamicIndexBufferBgfx()
    : BufferBase(Buffer::TYPE_INDEX, Buffer::USAGE_DYNAMIC)
{
}

uint64_t DynamicIndexBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc DynamicIndexBufferBgfx::recycle()
{
    return _handle.recycle();
}

void DynamicIndexBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    if(!_handle)
        _handle.reset(::bgfx::createDynamicIndexBuffer(_size));

    DASSERT(_size <= _indices.size());
    ::bgfx::update(_handle, 0, ::bgfx::copy(_indices.data(), _size));
}

void DynamicIndexBufferBgfx::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    _indices = UploaderType::toBytes(input);
    _size = _indices.size();
    upload(graphicsContext);
}

void DynamicIndexBufferBgfx::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    FATAL("Unimplemented");
}

void DynamicIndexBufferBgfx::bind()
{
    ::bgfx::setIndexBuffer(_handle);
}

void DynamicIndexBufferBgfx::bindRange(uint32_t first, uint32_t count)
{
    ::bgfx::setIndexBuffer(_handle, first, count);
}

}
