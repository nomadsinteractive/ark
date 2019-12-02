#include "renderer/base/buffer.h"

#include <string.h>

#include "core/base/memory_pool.h"

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
    : _delegate(stub), _uploader(uploader), _size(_uploader? _uploader->size() : 0)
{
}

Buffer::Snapshot::operator bool() const
{
    return static_cast<bool>(_delegate);
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
    _delegate->upload(graphicsContext, _uploader);
}

const sp<Buffer::Delegate>& Buffer::Snapshot::delegate() const
{
    return _delegate;
}

Buffer::Buffer(const sp<Buffer::Delegate>& delegate) noexcept
    : _delegate(delegate)
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
    _delegate->upload(graphicsContext, nullptr);
}

const sp<Buffer::Delegate>& Buffer::delegate() const
{
    return _delegate;
}

Buffer::Builder::Builder(size_t stride, size_t growCapacity)
    : _stride(stride), _grow_capacity(growCapacity), _ptr(nullptr), _boundary(nullptr), _size(0)
{
}

void Buffer::Builder::writeArray(ByteArray& array)
{
    DCHECK(array.length() <= _stride, "Varyings buffer overflow: stride: %d, varyings size: %d", _stride, array.length());
    DCHECK(_ptr + _stride <= _boundary, "Varyings buffer out of bounds");
    memcpy(_ptr, array.buf(), array.length());
}

void Buffer::Builder::setGrowCapacity(size_t growCapacity)
{
    _grow_capacity = growCapacity;
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

Buffer::Snapshot Buffer::Builder::toSnapshot(const Buffer& buffer) const
{
    return buffer.snapshot(makeUploader());
}

sp<Uploader> Buffer::Builder::makeUploader() const
{
    if(_buffers.size() == 0)
        return nullptr;

    if(_buffers.size() == 1)
        return sp<Uploader>::adopt(new ByteArrayUploader(_buffers[0]));

    return sp<Uploader>::adopt(new ByteArrayListUploader(_buffers));
}

size_t Buffer::Builder::stride() const
{
    return _stride;
}

size_t Buffer::Builder::length() const
{
    return _size / _stride;
}

void Buffer::Builder::grow()
{
    if(_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = sp<ByteArray::Allocated>::make(_grow_capacity * _stride);
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

Buffer::Delegate::Delegate()
    :_size(0)
{
}

size_t Buffer::Delegate::size() const
{
    return _size;
}

}
