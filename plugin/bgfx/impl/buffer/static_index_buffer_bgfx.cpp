#include "bgfx/impl/buffer/static_index_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

StaticIndexBufferBgfx::StaticIndexBufferBgfx()
    : BufferBase(Buffer::Usage(Buffer::USAGE_BIT_INDEX))
{
}

uint64_t StaticIndexBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc StaticIndexBufferBgfx::recycle()
{
    return _handle.recycle();
}

void StaticIndexBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    if(_handle)
        _handle.destroy();

    DASSERT(_size <= _indices.size());
    _handle.reset(::bgfx::createIndexBuffer(::bgfx::copy(_indices.data(), _size)));
}

void StaticIndexBufferBgfx::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    CHECK(!::bgfx::isValid(_handle), "Cannot upload data into a static index buffer");
    _indices = UploaderType::toBytes(input);
    _size = _indices.size();
    upload(graphicsContext);
}

void StaticIndexBufferBgfx::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    FATAL("Unimplemented");
}

void StaticIndexBufferBgfx::bind()
{
    ::bgfx::setIndexBuffer(_handle);
}

void StaticIndexBufferBgfx::bindRange(uint32_t first, uint32_t count)
{
    ::bgfx::setIndexBuffer(_handle, first, count);
}

}
