#include "renderer/base/buffer.h"

#include <string.h>

#include "core/base/memory_pool.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/uploader.h"

namespace ark {

namespace {

class SnapshotUploader : public Uploader {
public:
    SnapshotUploader(size_t size, std::vector<Buffer::Strip> strips)
        : Uploader(size), _blocks(std::move(strips)) {
    }

    virtual void upload(Writable& uploader) override {
        for(const auto& [i, j] : _blocks)
            uploader.write(j.buf(), static_cast<uint32_t>(j.length()), static_cast<uint32_t>(i));
    }

private:
    std::vector<std::pair<size_t, ByteArray::Borrowed>> _blocks;
};

class UploaderByInput : public Uploader {
public:
    UploaderByInput(sp<Input> input)
        : Uploader(input->size()), _input(std::move(input)) {

    }

    virtual void upload(Writable& writable) override {
        std::vector<int8_t> buf(_size);
        _input->flat(buf.data());
        writable.write(buf.data(), _size, 0);
    }

private:
    sp<Input> _input;
};

class BufferObjectUploader : public Uploader {
public:
    [[deprecated]]
    BufferObjectUploader(std::vector<sp<Input>> vars, size_t stride, size_t length)
        : Uploader(stride * length), _stride(stride), _length(length), _vars(std::move(vars)) {
    }

    virtual void upload(Writable& writable) override {
        std::vector<int8_t> buf(_stride);
        for(size_t i = 0; i < _length; ++i) {
            size_t offset = 0;
            for(const sp<Input>& j : _vars) {
                j->flat(&buf[offset]);
                offset += j->size();
            }
            writable.write(&buf[0], _stride, _stride * i);
        }
    }

private:
    size_t calcUploaderSize(const std::vector<sp<Input>>& vars) const {
        size_t size = 0;
        for(const sp<Input>& i : vars)
            size += i->size();
        return size;
    }

private:
    size_t _stride;
    size_t _length;
    std::vector<sp<Input>> _vars;
};

class PreRenderUpdate : public Updatable {
public:
    PreRenderUpdate(Buffer buffer, sp<Uploader> uploader, const sp<RenderController>& renderController, sp<Updatable> updatable)
        : _buffer(std::move(buffer)), _uploader(std::move(uploader)), _render_controller(renderController), _updatable(std::move(updatable)) {
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
    sp<Uploader> _uploader;
    sp<RenderController> _render_controller;
    sp<Updatable> _updatable;
};

}

Buffer::Snapshot::Snapshot(sp<Delegate> stub)
    : _delegate(std::move(stub)), _size(_delegate->size())
{
}

Buffer::Snapshot::Snapshot(sp<Delegate> stub, size_t size)
    : _delegate(std::move(stub)), _size(size)
{
}

Buffer::Snapshot::Snapshot(sp<Delegate> stub, sp<Uploader> uploader)
    : _delegate(std::move(stub)), _uploader(std::move(uploader)), _size(_uploader? _uploader->size() : 0)
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
    _delegate->setUploader(_uploader);
    _delegate->upload(graphicsContext);
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

Buffer::operator bool() const
{
    return static_cast<bool>(_delegate);
}

Buffer::Snapshot Buffer::snapshot(const ByteArray::Borrowed& strip) const
{
    return Snapshot(_delegate, sp<SnapshotUploader>::make(strip.length(), std::vector<Buffer::Strip>{{0, strip}}));
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

Buffer::Factory::Factory(size_t stride)
    : _stride(stride), _size(0)
{
}

Buffer::Snapshot Buffer::Factory::toSnapshot(const Buffer& buffer)
{
    if(_strips.size() == 0)
        return buffer.snapshot(nullptr);

    return buffer.snapshot(sp<SnapshotUploader>::make(_size, std::move(_strips)));
}

void Buffer::Factory::addStrip(size_t offset, ByteArray::Borrowed& content)
{
    _strips.emplace_back(offset, content);
    _size = std::max(_size, content.length() + offset);
}

Uploader::Uploader(size_t size)
    : _size(size)
{
}

size_t Uploader::size()
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

void Buffer::Delegate::setUploader(sp<Uploader> uploader)
{
    _uploader = std::move(uploader);
}

Buffer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _input(factory.getBuilder<Input>(manifest, Constants::Attributes::INPUT)), _length(factory.getBuilder<Integer>(manifest, "length")),
      _stride(factory.getBuilder<Integer>(manifest, "stride")), _usage(Documents::getAttribute<Usage>(manifest, "usage", USAGE_DYNAMIC))
{
    DCHECK(_input || _length, "You must specify either \"%s\" or \"length\" to define a buffer", Constants::Attributes::INPUT);
    for(const document& i : manifest->children())
    {
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        _vars.push_back(factory.ensureBuilderByTypeValue<Input>(type, value));
    }
}

sp<Buffer> Buffer::BUILDER::build(const Scope& args)
{
    size_t stride = 0;
    std::vector<sp<Input>> vars;
    for(const sp<Builder<Input>>& i : _vars)
    {
        sp<Input> var = i->build(args);
        stride += var->size();
        vars.push_back(std::move(var));
    }
    const sp<RenderController>& renderController = _resource_loader_context->renderController();
    sp<Input> input = _input ? _input->build(args) : nullptr;
    sp<Uploader> uploader = input ? sp<Uploader>::make<UploaderByInput>(input)
                                     : sp<Uploader>::make<BufferObjectUploader>(std::move(vars), _stride ? static_cast<uint32_t>(_stride->build(args)->val()) : stride, _length->build(args)->val());
    sp<Buffer> buffer = sp<Buffer>::make(renderController->makeBuffer(Buffer::TYPE_STORAGE, _usage, uploader));
    if(input)
        renderController->addPreRenderUpdateRequest(sp<PreRenderUpdate>::make(buffer, std::move(uploader), renderController, input), sp<BooleanByWeakRef<Buffer::Delegate>>::make(buffer->delegate(), 2));
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
