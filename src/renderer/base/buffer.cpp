#include "renderer/base/buffer.h"

#include "core/ark.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class UploaderBufferSnapshot final : public Uploader {
public:
    UploaderBufferSnapshot(const size_t size, Vector<Buffer::Strip> strips)
        : Uploader(size), _strips(std::move(strips)) {
    }

    void upload(Writable& uploader) override
    {
        for(const auto& [i, j] : _strips)
            uploader.write(j.buf(), static_cast<uint32_t>(j.length()), static_cast<uint32_t>(i));
    }

    bool update(uint32_t /*tick*/) override {
        return false;
    }

private:
    Vector<std::pair<size_t, ByteArray::Borrowed>> _strips;
};

class RunnableBufferSynchronizer final : public Runnable {
public:
    RunnableBufferSynchronizer(Buffer buffer, sp<ByteArray> memory, const size_t offset)
        : _buffer(std::move(buffer)), _memory(std::move(memory)), _offset(offset)
    {
    }

    void run() override
    {
        if(_buffer.id())
            _buffer.delegate()->downloadBuffer(GraphicsContext::mocked(), _offset, _memory->size(), _memory->buf());
    }

private:
    Buffer _buffer;
    sp<ByteArray> _memory;
    size_t _offset;
};

}

Buffer::Snapshot::Snapshot(sp<Delegate> stub)
    : _delegate(std::move(stub)), _size(_delegate->size())
{
}

Buffer::Snapshot::Snapshot(sp<Delegate> stub, const size_t size, sp<Uploader> uploader)
    : _delegate(std::move(stub)), _uploader(std::move(uploader)), _size(size)
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
    if(_uploader)
        _delegate->uploadBuffer(graphicsContext, _uploader);
}

const sp<Buffer::Delegate>& Buffer::Snapshot::delegate() const
{
    return _delegate;
}

Buffer::Buffer(const UsageBits usageBits, sp<Uploader> uploader)
    : _delegate(Ark::instance().renderController()->makeBuffer(Usage(usageBits), std::move(uploader))._delegate)
{
}

Buffer::Buffer(sp<Buffer::Delegate> delegate) noexcept
    : _delegate(std::move(delegate))
{
}

size_t Buffer::size() const
{
    return _delegate ? _delegate->size() : 0;
}

Buffer::Snapshot Buffer::snapshot(size_t size) const
{
    return {_delegate, size, nullptr};
}

Buffer::operator bool() const
{
    return static_cast<bool>(_delegate);
}

Buffer::Snapshot Buffer::snapshot(const ByteArray::Borrowed& strip) const
{
    return {_delegate, strip.length(), sp<UploaderBufferSnapshot>::make(strip.length(), Vector<Buffer::Strip>{{0, strip}})};
}

Buffer::Snapshot Buffer::snapshot(sp<Uploader> uploader, size_t size) const
{
    return {_delegate, size, std::move(uploader)};
}

uint64_t Buffer::id() const
{
    return _delegate->id();
}

sp<ByteArray> Buffer::synchronize(const size_t offset, const size_t size, sp<Boolean> canceled)
{
    sp<ByteArray> memory = sp<ByteArray>::make<ByteArray::Allocated>(size);
    RenderController& renderController = Ark::instance().renderController();
    memset(memory->buf(), 0, size);
    renderController.addPreRenderRequest(sp<Runnable>::make<RunnableBufferSynchronizer>(*this, memory, offset), std::move(canceled));
    return memory;
}

void Buffer::upload(GraphicsContext& graphicsContext) const
{
    _delegate->upload(graphicsContext);
}

const sp<Buffer::Delegate>& Buffer::delegate() const
{
    return _delegate;
}

Buffer::SnapshotFactory::SnapshotFactory(const size_t stride)
    : _stride(stride), _size(0)
{
}

Buffer::Snapshot Buffer::SnapshotFactory::toSnapshot(const Buffer& buffer)
{
    if(_strips.empty())
        return buffer.snapshot();

    return buffer.snapshot(sp<Uploader>::make<UploaderBufferSnapshot>(_size, std::move(_strips)));
}

void Buffer::SnapshotFactory::addStrip(const size_t offset, ByteArray::Borrowed& content)
{
    _strips.emplace_back(offset, content);
    _size = std::max(_size, content.length() + offset);
}

size_t Buffer::Delegate::size() const
{
    return _size;
}

void Buffer::Delegate::setSize(const size_t size)
{
    _size = size;
}

Buffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _uploader(factory.getBuilder<Uploader>(manifest, constants::UPLOADER)), _usage(Documents::getAttribute<Usage>(manifest, "usage", {USAGE_BIT_STORAGE, USAGE_BIT_DYNAMIC}))
{
}

sp<Buffer> Buffer::BUILDER::build(const Scope& args)
{
    return sp<Buffer>::make(Ark::instance().renderController()->makeBuffer(_usage, _uploader.build(args)));
}

template<> ARK_API Buffer::Usage StringConvert::eval<Buffer::Usage>(const String& expr)
{
    constexpr Buffer::Usage::LookupTable<7> bits = {{
        {"vertex", Buffer::USAGE_BIT_VERTEX},
        {"index", Buffer::USAGE_BIT_INDEX},
        {"indirect", Buffer::USAGE_BIT_DRAW_INDIRECT},
        {"storage", Buffer::USAGE_BIT_STORAGE},
        {"dynamic", Buffer::USAGE_BIT_DYNAMIC},
        {"transfer_src", Buffer::USAGE_BIT_TRANSFER_SRC},
        {"host_visible", Buffer::USAGE_BIT_HOST_VISIBLE}
    }};
    return Buffer::Usage::toBitSet(expr, bits);
}

}
