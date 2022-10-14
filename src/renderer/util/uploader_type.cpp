#include "renderer/util/uploader_type.h"

#include <vector>

#include "core/base/bean_factory.h"
#include "core/impl/writable/writable_memory.h"
#include "core/inf/input.h"
#include "core/inf/variable.h"

#include "renderer/inf/uploader.h"
#include "renderer/impl/uploader/uploader_impl.h"
#include "renderer/impl/uploader/uploader_recoder.h"
#include "renderer/impl/uploader/uploader_wrapper.h"

namespace ark {

namespace {

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

}

static sp<UploaderImpl> ensureUploaderImpl(const sp<Uploader>& self)
{
    sp<UploaderImpl> uploaderImpl = self.as<UploaderImpl>();
    CHECK(uploaderImpl, "Self is not an UploaderImpl instance");
    return uploaderImpl;
}

sp<Uploader> UploaderType::create(sp<Input> value, size_t size)
{
    std::map<size_t, sp<Input>> inputs;
    inputs.insert(std::make_pair(0, std::move(value)));
    return sp<UploaderImpl>::make(std::move(inputs), size);
}

sp<Uploader> UploaderType::create(std::map<size_t, sp<Input>> inputs, size_t size)
{
    return sp<UploaderImpl>::make(std::move(inputs), size);
}

size_t UploaderType::size(const sp<Uploader>& self)
{
    return self->size();
}

bytearray UploaderType::toBytes(const sp<Uploader>& self)
{
    std::vector<uint8_t> buf(self->size());
    self->upload(WritableMemory(buf.data()));
    return sp<ByteArray::Vector>::make(std::move(buf));
}

void UploaderType::set(const sp<Uploader>& self, sp<Uploader> delegate)
{
    const sp<UploaderWrapper> uploaderWrapper = self.as<UploaderWrapper>();
    CHECK(uploaderWrapper, "Self is not an UploaderWrapper instance");
    uploaderWrapper->reset(delegate);
}

sp<Uploader> UploaderType::wrap(sp<Uploader> self)
{
    return sp<UploaderWrapper>::make(std::move(self));
}

sp<Uploader> UploaderType::record(const sp<Uploader>& self)
{
    return sp<UploaderRecorder>::make(self);
}

void UploaderType::addInput(const sp<Uploader>& self, size_t offset, sp<Input> input)
{
    ensureUploaderImpl(self)->addInput(offset, std::move(input));
}

void UploaderType::removeInput(const sp<Uploader>& self, size_t offset)
{
    ensureUploaderImpl(self)->removeInput(offset);
}

UploaderType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _input(factory.getBuilder<Input>(manifest, Constants::Attributes::INPUT)), _length(factory.getBuilder<Integer>(manifest, "length")),
      _stride(factory.getBuilder<Integer>(manifest, "stride"))
{
    DCHECK(_input || _length, "You must specify either \"%s\" or \"length\" to define a Uploader", Constants::Attributes::INPUT);
    for(const document& i : manifest->children("attribute"))
    {
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        _attributes.push_back(factory.ensureBuilderByTypeValue<Input>(type, value));
    }
}

sp<Uploader> UploaderType::BUILDER::build(const Scope& args)
{
    size_t stride = 0;
    std::vector<sp<Input>> vars;
    for(const sp<Builder<Input>>& i : _attributes)
    {
        sp<Input> var = i->build(args);
        stride += var->size();
        vars.push_back(std::move(var));
    }
    sp<Input> input = _input ? _input->build(args) : nullptr;
    return input ? sp<Uploader>::make<UploaderImpl>(std::map<size_t, sp<Input>>{{0, input}})
                                    : sp<Uploader>::make<BufferObjectUploader>(std::move(vars), _stride ? static_cast<uint32_t>(_stride->build(args)->val()) : stride, _length->build(args)->val());
}

}
