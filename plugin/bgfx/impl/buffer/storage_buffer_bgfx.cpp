#include "storage_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

StorageBufferBgfx::StorageBufferBgfx()
    : BufferBase({Buffer::USAGE_BIT_STORAGE, Buffer::USAGE_BIT_DYNAMIC})
{
}

uint64_t StorageBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc StorageBufferBgfx::recycle()
{
    return _handle.recycle();
}

void StorageBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    DASSERT(_size <= _data.size());
    if(_handle)
        ::bgfx::update(_handle, 0, ::bgfx::copy(_data.data(), _size));
    else
        _handle.reset(::bgfx::createDynamicIndexBuffer(::bgfx::copy(_data.data(), _size)));

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
    ::bgfx::setBuffer(0, _handle, ::bgfx::Access::ReadWrite);
}

void StorageBufferBgfx::bindRange(uint32_t first, uint32_t count)
{
    ::bgfx::setBuffer(0, _handle, ::bgfx::Access::ReadWrite);
}

}
