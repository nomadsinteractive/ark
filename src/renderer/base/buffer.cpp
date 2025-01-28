#include "renderer/base/buffer.h"

#include "core/ark.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class InputBufferSnapshot final : public Uploader {
public:
    InputBufferSnapshot(size_t size, std::vector<Buffer::Strip> strips)
        : Uploader(size), _blocks(std::move(strips)) {
    }

    void upload(Writable& uploader) override {
        for(const auto& [i, j] : _blocks)
            uploader.write(j.buf(), static_cast<uint32_t>(j.length()), static_cast<uint32_t>(i));
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    std::vector<std::pair<size_t, ByteArray::Borrowed>> _blocks;
};

class RunnableBufferSynchronizer final : public Runnable {
public:
    RunnableBufferSynchronizer(Buffer buffer, sp<ByteArray> memory, size_t offset)
        : _buffer(std::move(buffer)), _memory(std::move(memory)), _offset(offset) {
    }

    void run() override {
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

Buffer::Snapshot::Snapshot(sp<Delegate> stub, size_t size, sp<Uploader> input)
    : _delegate(std::move(stub)), _uploader(std::move(input)), _size(size)
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
    return Snapshot(_delegate, strip.length(), sp<InputBufferSnapshot>::make(strip.length(), std::vector<Buffer::Strip>{{0, strip}}));
}

Buffer::Snapshot Buffer::snapshot(sp<Uploader> input, size_t size) const
{
    return {_delegate, size, std::move(input)};
}

uint64_t Buffer::id() const
{
    return _delegate->id();
}

sp<ByteArray> Buffer::synchronize(size_t offset, size_t size, sp<Boolean> cancelled)
{
    sp<ByteArray> memory = sp<ByteArray::Allocated>::make(size);
    RenderController& renderController = Ark::instance().renderController();
    renderController.addPreRenderRequest(sp<RunnableBufferSynchronizer>::make(*this, memory, offset), std::move(cancelled));
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

Buffer::Factory::Factory(size_t stride)
    : _stride(stride), _size(0)
{
}

Buffer::Snapshot Buffer::Factory::toSnapshot(const Buffer& buffer)
{
    if(_strips.empty())
        return buffer.snapshot();

    return buffer.snapshot(sp<InputBufferSnapshot>::make(_size, std::move(_strips)));
}

void Buffer::Factory::addStrip(size_t offset, ByteArray::Borrowed& content)
{
    _strips.emplace_back(offset, content);
    _size = std::max(_size, content.length() + offset);
}

size_t Buffer::Delegate::size() const
{
    return _size;
}

void Buffer::Delegate::setSize(size_t size)
{
    _size = size;
}

Buffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _uploader(factory.getBuilder<Uploader>(manifest, "uploader")), _usage(Documents::getAttribute<Usage>(manifest, "usage", USAGE_BIT_DYNAMIC))
{
}

sp<Buffer> Buffer::BUILDER::build(const Scope& args)
{
    return sp<Buffer>::make(Ark::instance().renderController()->makeBuffer(Buffer::TYPE_STORAGE, _usage, _uploader.build(args)));
}

template<> ARK_API Buffer::Usage StringConvert::eval<Buffer::Usage>(const String& str)
{
    constexpr std::array<std::pair<const char*, Buffer::UsageBit>, 2> bits = {{{"dynamic", Buffer::USAGE_BIT_DYNAMIC}, {"host_visible", Buffer::USAGE_BIT_HOST_VISIBLE}}};
    return Buffer::Usage::toBitSet(str, bits);
}

}
