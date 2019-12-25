#include "renderer/base/buffer.h"

#include <string.h>

#include "core/base/memory_pool.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/uploader.h"

namespace ark {

namespace {

class UploaderImpl : public Uploader {
public:
    UploaderImpl(const std::vector<Buffer::Block>& blocks)
        : Uploader(getUploaderSize(blocks)), _blocks(blocks) {

    }

    virtual void upload(const UploadFunc& uploader) override {
        for(const Buffer::Block& i : _blocks)
            uploader(i.content.buf(), i.content.length(), i.offset);
    }

private:
    static size_t getUploaderSize(const std::vector<Buffer::Block>& blocks) {
        size_t size = 0;
        for(const Buffer::Block& i : blocks)
            size = std::max(size, i.offset + i.content.length());
        return size;
    }

private:
    std::vector<Buffer::Block> _blocks;
};

}

Buffer::Attributes::Attributes(const PipelineInput& input)
{
    _offsets[ATTRIBUTE_NAME_TEX_COORDINATE] = input.getAttributeOffset("TexCoordinate");
    _offsets[ATTRIBUTE_NAME_NORMAL] = input.getAttributeOffset("Normal");
    _offsets[ATTRIBUTE_NAME_TANGENT] = input.getAttributeOffset("Tangent");
    _offsets[ATTRIBUTE_NAME_BITANGENT] = input.getAttributeOffset("Bitangent");
    _offsets[ATTRIBUTE_NAME_MODEL_ID] = input.getAttributeOffset("ModelId");
}

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

Buffer::Builder::Builder(const RenderRequest& renderRequest, const Attributes& attributes, size_t stride)
    : _render_request(renderRequest), _attributes(attributes), _stride(stride), _grow_capacity(0), _ptr(nullptr), _boundary(nullptr), _size(0)
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

void Buffer::Builder::addBlock(size_t offset, ByteArray::Borrowed& content)
{
    _blocks.emplace_back(offset, content);
    _size = std::max(_size, content.length() + offset);
}

sp<Uploader> Buffer::Builder::makeUploader() const
{
    if(_blocks.size() > 0)
        return sp<UploaderImpl>::make(_blocks);

    if(_buffers.size() == 0)
        return nullptr;

    if(_buffers.size() == 1)
        return sp<Uploader>::make<ByteArrayUploader>(_buffers.at(0));

    return sp<Uploader>::make<ByteArrayListUploader>(_buffers);
}

size_t Buffer::Builder::length() const
{
    return _size / _stride;
}

//BufferWriter Buffer::Builder::makeWriter(size_t size, size_t offset)
//{
//    Block block(offset, _render_request.allocator().sbrk(size));
//    BufferWriter writer(_attributes, block.content.buf(), block.content.length(), _stride);
//    _blocks.push_back(block);
//    _size = std::max(_size, size + offset);
//    return writer;
//}

void Buffer::Builder::grow()
{
    if(_buffers.size() % 4 == 3)
        _grow_capacity *= 2;

    const bytearray bytes = sp<ByteArray::Borrowed>::make(_render_request.allocator().sbrk(_grow_capacity * _stride));
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

Buffer::Block::Block(size_t offset, const ByteArray::Borrowed& content)
    : offset(offset), content(content)
{
}

}
