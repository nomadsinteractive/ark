#include "renderer/base/buffer.h"

#include "core/ark.h"
#include "core/base/memory_pool.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class InputSnapshot : public Input {
public:
    InputSnapshot(size_t size, std::vector<Buffer::Strip> strips)
        : Input(size), _blocks(std::move(strips)) {
    }

    virtual void upload(Writable& uploader) override {
        for(const auto& [i, j] : _blocks)
            uploader.write(j.buf(), static_cast<uint32_t>(j.length()), static_cast<uint32_t>(i));
    }

private:
    std::vector<std::pair<size_t, ByteArray::Borrowed>> _blocks;
};

}

Buffer::Snapshot::Snapshot(sp<Delegate> stub)
    : _delegate(std::move(stub)), _size(_delegate->size())
{
}

Buffer::Snapshot::Snapshot(sp<Delegate> stub, size_t size, sp<Input> input)
    : _delegate(std::move(stub)), _input(std::move(input)), _size(size)
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
    if(_input)
        _delegate->uploadBuffer(graphicsContext, _input);
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
    return _delegate->size();
}

Buffer::Snapshot Buffer::snapshot(size_t size) const
{
    return Snapshot(_delegate, size, nullptr);
}

Buffer::operator bool() const
{
    return static_cast<bool>(_delegate);
}

Buffer::Snapshot Buffer::snapshot(const ByteArray::Borrowed& strip) const
{
    return Snapshot(_delegate, strip.length(), sp<InputSnapshot>::make(strip.length(), std::vector<Buffer::Strip>{{0, strip}}));
}

Buffer::Snapshot Buffer::snapshot(sp<Input> input, size_t size) const
{
    return Snapshot(_delegate, size, std::move(input));
}

uint64_t Buffer::id() const
{
    return _delegate->id();
}

//void Buffer::upload(sp<Uploader> uploader, sp<Future> future)
//{
//    Ark::instance().applicationContext()->renderController()->uploadBuffer(*this, std::move(uploader), RenderController::US_ON_CHANGE, std::move(future));
//}

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
    if(_strips.size() == 0)
        return buffer.snapshot();

    return buffer.snapshot(sp<InputSnapshot>::make(_size, std::move(_strips)));
}

void Buffer::Factory::addStrip(size_t offset, ByteArray::Borrowed& content)
{
    _strips.emplace_back(offset, content);
    _size = std::max(_size, content.length() + offset);
}

Buffer::Delegate::Delegate()
    :_size(0)
{
}

size_t Buffer::Delegate::size() const
{
    return _size;
}

Buffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _input(factory.getBuilder<Input>(manifest, "input")), _usage(Documents::getAttribute<Usage>(manifest, "usage", USAGE_DYNAMIC))
{
}

sp<Buffer> Buffer::BUILDER::build(const Scope& args)
{
    const sp<RenderController>& renderController = _resource_loader_context->renderController();
    return sp<Buffer>::make(renderController->makeBuffer(Buffer::TYPE_STORAGE, _usage, _input->build(args)));
}

template<> ARK_API Buffer::Usage Conversions::to<String, Buffer::Usage>(const String& str)
{
    if(str == "dynamic")
        return Buffer::USAGE_DYNAMIC;
    DCHECK(str == "static", "Unknown BufferUsage: \"%s\", possible values are [dynamic, static]", str.c_str());
    return Buffer::USAGE_STATIC;
}

}
