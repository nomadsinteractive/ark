#include "renderer/base/buffer.h"

#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"

#include "renderer/inf/uploader.h"

namespace ark {

Buffer::Snapshot::Snapshot(const sp<Delegate>& stub)
    : _delegate(stub), _size(stub->size())
{
}

Buffer::Snapshot::Snapshot(const sp<Delegate>& stub, size_t size)
    : _delegate(stub), _size(size)
{
}

Buffer::Snapshot::Snapshot(const sp<Delegate>& stub, const sp<Uploader>& uploader)
    : _delegate(stub), _uploader(uploader), _size(uploader->size())
{
}

uint64_t Buffer::Snapshot::id() const
{
    return _delegate->id();
}

size_t Buffer::Snapshot::size() const
{
    return _size;
}

void Buffer::Snapshot::upload(GraphicsContext& graphicsContext) const
{
    _delegate->reload(graphicsContext, _uploader);
}

Buffer::Buffer(const sp<Buffer::Delegate>& delegate)
    : _delegate(delegate)
{
}

Buffer::Buffer() noexcept
    : _delegate(nullptr)
{
}

size_t Buffer::size() const
{
    return _delegate->size();
}

Buffer::operator bool() const
{
    return static_cast<bool>(_delegate);
}

Buffer::Snapshot Buffer::snapshot(const sp<Uploader>& uploader) const
{
    return Snapshot(_delegate, uploader);
}

Buffer::Snapshot Buffer::snapshot(size_t size) const
{
    return Snapshot(_delegate, size);
}

Buffer::Snapshot Buffer::snapshot() const
{
    return Snapshot(_delegate);
}

uint64_t Buffer::id() const
{
    return _delegate->id();
}

void Buffer::upload(GraphicsContext& graphicsContext) const
{
    _delegate->upload(graphicsContext);
}

const sp<Buffer::Delegate>& Buffer::delegate() const
{
    return _delegate;
}

Buffer::Builder::Builder(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t stride, size_t growCapacity)
    : _memory_pool(memoryPool), _object_pool(objectPool), _stride(stride), _grow_capacity(growCapacity), _ptr(nullptr), _boundary(nullptr), _size(0)
{
}

void Buffer::Builder::setGrowCapacity(size_t growCapacity)
{
    _grow_capacity = growCapacity;
}

void Buffer::Builder::apply(const bytearray& buf)
{
    if(!buf)
        return;

    DCHECK(buf->length() <= _stride, "Varyings buffer overflow: stride: %d, varyings size: %d", _stride, buf->length());
    DCHECK(_ptr + _stride <= _boundary, "Varyings buffer out of bounds");
    memcpy(_ptr, buf->buf(), buf->length());
}

void Buffer::Builder::next()
{
    if(_ptr == _boundary)
        grow();
    else
        _ptr += _stride;

    _size += _stride;
    DCHECK(_ptr <= _boundary, "Array buffer out of bounds");
}

sp<Uploader> Buffer::Builder::makeUploader() const
{
    if(_buffers.size() == 1)
        return _object_pool->obtain<ByteArrayUploader>(_buffers[0]);
    return _object_pool->obtain<ByteArrayListUploader>(_buffers);
}

size_t Buffer::Builder::stride() const
{
    return _stride;
}

size_t Buffer::Builder::size() const
{
    return _size;
}

void Buffer::Builder::grow()
{
    if(_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = _memory_pool->allocate(_grow_capacity * _stride);
    _buffers.push_back(bytes);
    _ptr = bytes->buf();
    _boundary = _ptr + bytes->length();
}

Uploader::Uploader(size_t size)
    : _size(size)
{
}

size_t Uploader::size() const
{
    return _size;
}

Buffer::Delegate::Delegate(size_t size)
    : _size(size)
{
}

size_t Buffer::Delegate::size() const
{
    return _size;
}

}
