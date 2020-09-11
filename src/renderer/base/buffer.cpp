#include "renderer/base/buffer.h"

#include <string.h>

#include "core/base/memory_pool.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/uploader.h"

namespace ark {

namespace {

class UploaderImpl : public Uploader {
public:
    UploaderImpl(const std::vector<Buffer::Block>& blocks)
        : Uploader(getUploaderSize(blocks)), _blocks(blocks) {

    }

    virtual void upload(Writable& uploader) override {
        for(const Buffer::Block& i : _blocks)
            uploader.write(i.content.buf(), static_cast<uint32_t>(i.content.length()), static_cast<uint32_t>(i.offset));
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

class UploaderByFlatable : public Uploader {
public:
    UploaderByFlatable(sp<Flatable> flatable)
        : Uploader(flatable->size()), _flatable(std::move(flatable)) {

    }

    virtual void upload(Writable& writable) override {
        std::vector<int8_t> buf(_size);
        _flatable->flat(buf.data());
        writable.write(buf.data(), _size, 0);
    }

private:
    sp<Flatable> _flatable;
};

class BufferObjectUploader : public Uploader {
public:
    BufferObjectUploader(std::vector<sp<Flatable>> vars, size_t stride, size_t length)
        : Uploader(stride * length), _stride(stride), _length(length), _vars(std::move(vars)) {
    }

    virtual void upload(Writable& writable) override {
        std::vector<int8_t> buf(_stride);
        for(size_t i = 0; i < _length; ++i) {
            size_t offset = 0;
            for(const sp<Flatable>& j : _vars) {
                j->flat(&buf[offset]);
                offset += j->size();
            }
            writable.write(&buf[0], _stride, _stride * i);
        }
    }

private:
    size_t calcUploaderSize(const std::vector<sp<Flatable>>& vars) const {
        size_t size = 0;
        for(const sp<Flatable>& i : vars)
            size += i->size();
        return size;
    }

private:
    size_t _stride;
    size_t _length;
    std::vector<sp<Flatable>> _vars;
};

class PreRenderUpdate : public Updatable {
public:
    PreRenderUpdate(Buffer buffer, const sp<RenderController>& renderController, sp<Uploader> uploader, sp<Updatable> updatable)
        : _buffer(std::move(buffer)), _render_controller(renderController), _uploader(std::move(uploader)), _updatable(std::move(updatable)) {
    }

    virtual bool update(uint64_t timestamp) override {
        if(_updatable->update(timestamp)) {
            _render_controller->uploadBuffer(_buffer, _uploader, RenderController::US_ONCE);
            return true;
        }
        return false;
    }

private:
    Buffer _buffer;

    sp<RenderController> _render_controller;
    sp<Uploader> _uploader;
    sp<Updatable> _updatable;
};

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

Buffer::Factory::Factory(size_t stride)
    : _stride(stride), _size(0)
{
}

Buffer::Snapshot Buffer::Factory::toSnapshot(const Buffer& buffer) const
{
    return buffer.snapshot(makeUploader());
}

void Buffer::Factory::addBlock(size_t offset, ByteArray::Borrowed& content)
{
    _blocks.emplace_back(offset, content);
    _size = std::max(_size, content.length() + offset);
}

sp<Uploader> Buffer::Factory::makeUploader() const
{
    if(_blocks.size() == 0)
        return nullptr;

    return sp<UploaderImpl>::make(_blocks);
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

Buffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _flatable(factory.getBuilder<Flatable>(manifest, "data")), _length(factory.getBuilder<Integer>(manifest, "length")),
      _stride(factory.getBuilder<Integer>(manifest, "stride")), _usage(Documents::getAttribute<Usage>(manifest, "usage", USAGE_DYNAMIC))
{
    DCHECK(_flatable || _length, "You must specify either \"data\" or \"length\" to define a buffer");
    for(const document& i : manifest->children())
    {
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        _vars.push_back(factory.ensureBuilderByTypeValue<Flatable>(type, value));
    }
}

sp<Buffer> Buffer::BUILDER::build(const Scope& args)
{
    size_t stride = 0;
    std::vector<sp<Flatable>> vars;
    for(const sp<Builder<Flatable>>& i : _vars)
    {
        sp<Flatable> var = i->build(args);
        stride += var->size();
        vars.push_back(std::move(var));
    }
    const sp<RenderController>& renderController = _resource_loader_context->renderController();
    sp<Flatable> flatable = _flatable ? _flatable->build(args) : nullptr;
    sp<Uploader> uploader = flatable ? sp<Uploader>::make<UploaderByFlatable>(flatable)
                                     : sp<Uploader>::make<BufferObjectUploader>(std::move(vars), _stride ? static_cast<uint32_t>(_stride->build(args)->val()) : stride, _length->build(args)->val());
    sp<Buffer> buffer = sp<Buffer>::make(renderController->makeBuffer(Buffer::TYPE_STORAGE, _usage, uploader));
    if(flatable)
        _resource_loader_context->renderController()->addPreRenderUpdateRequest(sp<PreRenderUpdate>::make(buffer, renderController, uploader, flatable), sp<BooleanByWeakRef<Buffer::Delegate>>::make(buffer->delegate(), 2));
    return buffer;
}

template<> ARK_API Buffer::Usage Conversions::to<String, Buffer::Usage>(const String& str)
{
    if(str == "dynamic")
        return Buffer::USAGE_DYNAMIC;
    DCHECK(str == "static", "Unknown BufferUsage: \"%s\", possible values are [dynamic, static]", str.c_str());
    return Buffer::USAGE_STATIC;
}

}
