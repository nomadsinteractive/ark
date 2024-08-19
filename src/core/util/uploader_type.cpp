#include "core/util/uploader_type.h"

#include "core/inf/array.h"
#include "core/impl/uploader/uploader_impl.h"
#include "core/impl/uploader/uploader_array.h"
#include "core/impl/uploader/input_repeat.h"
#include "core/impl/uploader/input_variable_array.h"
#include "core/impl/uploader/uploader_of_variable.h"
#include "core/impl/writable/writable_memory.h"
#include "core/impl/writable/writable_with_offset.h"
#include "core/impl/uploader/uploader_wrapper.h"

#include "graphics/base/v4.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class UploaderList final : public Uploader {
private:
    struct Node {
        size_t _offset;
        sp<Uploader> _uploader;
        bool _dirty;
    };

public:
    UploaderList(const std::vector<sp<Uploader>>& uploaders)
        : Uploader(0) {
        for(const sp<Uploader>& i : uploaders) {
            size_t size = i->size();
            _uploaders.push_back(Node{_size, i, true});
            _size += size;
        }
    }

    bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(Node& i : _uploaders) {
            i._dirty = i._uploader->update(timestamp) || i._dirty;
            if(i._dirty && !dirty)
                dirty = true;
        }
        return dirty;
    }

    void upload(Writable& buf) override {
        for(Node& i : _uploaders)
            if(i._dirty) {
                WritableWithOffset wwo(buf, i._offset);
                i._uploader->upload(wwo);
                i._dirty = false;
            }
    }

private:
    std::vector<Node> _uploaders;
};

struct WritableSnapshot final : Writable {
    WritableSnapshot(size_t size)
        : _size(size) {
    }

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        CHECK(offset + size <= _size, "Buffer overflow, buffer size: %zd, writing offset: %d, writing size: %d", _size, offset, size);
        sp<ByteArray> strip = sp<ByteArray::Allocated>::make(size);
        memcpy(strip->buf(), buffer, size);
        _strips.push_back(std::make_pair<size_t, sp<ByteArray>>(offset, std::move(strip)));
        return size;
    }

    size_t _size;
    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;
};

sp<UploaderImpl> ensureImpl(const sp<Uploader>& self)
{
    const sp<UploaderImpl> impl = self.tryCast<UploaderImpl>();
    CHECK(impl, "This object is not a InputImpl instance. Use \"reserve\" method to create an InputImpl instance.");
    return impl;
}

}


sp<Uploader> UploaderType::create(sp<ByteArray> value, size_t size)
{
    return reserve(sp<UploaderArray<uint8_t>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<IntArray> value, size_t size)
{
    return reserve(sp<UploaderArray<int32_t>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Integer> value, size_t size)
{
    return reserve(sp<UploaderOfVariable<int32_t>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Numeric> value, size_t size)
{
    return reserve(sp<UploaderOfVariable<float>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Vec2> value, size_t size)
{
    return reserve(sp<UploaderOfVariable<V2>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Vec3> value, size_t size)
{
    return reserve(sp<UploaderOfVariable<V3>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Vec4> value, size_t size)
{
    return reserve(sp<UploaderOfVariable<V4>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(std::map<size_t, sp<Uploader>> value, size_t size)
{
    return sp<UploaderImpl>::make(std::move(value), size);
}

sp<Uploader> UploaderType::create(std::vector<sp<Mat4>> value, size_t size)
{
    return reserve(sp<InputVariableArray<M4>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(const std::vector<sp<Uploader>>& value, size_t size)
{
    return reserve(sp<UploaderList>::make(value), size);
}

sp<Uploader> UploaderType::create(std::vector<V3> value, size_t size)
{
    return reserve(sp<UploaderArray<V3>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(std::vector<V4> value, size_t size)
{
    return reserve(sp<UploaderArray<V4>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(std::vector<uint32_t> value, size_t size)
{
    return reserve(sp<UploaderArray<uint32_t>>::make(std::move(value)), size);
}

sp<Uploader> UploaderType::create(const std::set<uint32_t>& value, size_t size)
{
    return reserve(sp<UploaderArray<uint32_t>>::make(std::vector<uint32_t>(value.begin(), value.end())), size);
}

std::vector<uint8_t> UploaderType::toBytes(Uploader& self)
{
    std::vector<uint8_t> bytes(self.size());
    WritableMemory writable(bytes.data());
    self.upload(writable);
    return bytes;
}

std::vector<std::pair<size_t, sp<ByteArray>>> UploaderType::record(Uploader& self)
{
    WritableSnapshot writable(self.size());
    self.upload(writable);
    return writable._strips;
}

sp<Uploader> UploaderType::wrap(sp<Uploader> self)
{
    return sp<UploaderWrapper>::make(std::move(self));
}

sp<Uploader> UploaderType::makeElementIndexInput(std::vector<element_index_t> value)
{
    return sp<UploaderArray<element_index_t>>::make(std::move(value));
}

void UploaderType::reset(const sp<Uploader>& self, sp<Uploader> delegate)
{
    if(const sp<UploaderImpl> impl = self.tryCast<UploaderImpl>())
        impl->reset(std::move(delegate));
    else
        ensureWrapper(self)->setDelegate(std::move(delegate));
}

size_t UploaderType::size(const sp<Uploader>& self)
{
    return self->size();
}

sp<Uploader> UploaderType::reserve(sp<Uploader> self, size_t size)
{
    if(size == 0)
        return self;

    ASSERT(size >= self->size());
    sp<UploaderImpl> inputImpl = sp<UploaderImpl>::make(size);
    if(self->size() > 0)
        inputImpl->addInput(0,  std::move(self));
    return inputImpl;
}

sp<Uploader> UploaderType::remap(sp<Uploader> self, size_t size, size_t offset)
{
    sp<UploaderImpl> inputImpl = sp<UploaderImpl>::make(size);
    if(self->size() > 0)
        inputImpl->addInput(offset,  std::move(self));
    return inputImpl;
}

sp<Uploader> UploaderType::repeat(sp<Uploader> self, size_t length, size_t stride)
{
    return sp<InputRepeat>::make(std::move(self), length, stride);
}

void UploaderType::addInput(const sp<Uploader>& self, size_t offset, sp<Uploader> input)
{
    ensureImpl(self)->addInput(offset, std::move(input));
}

void UploaderType::removeInput(const sp<Uploader>& self, size_t offset)
{
    ensureImpl(self)->removeInput(offset);
}

void UploaderType::markDirty(const sp<Uploader>& self)
{
    ensureImpl(self)->markDirty();
}

sp<UploaderWrapper> UploaderType::ensureWrapper(const sp<Uploader>& self)
{
    const sp<UploaderWrapper> wrapper = self.tryCast<UploaderWrapper>();
    CHECK(wrapper, "This Input object is not a InputWrapper instance");
    return wrapper;
}

}
