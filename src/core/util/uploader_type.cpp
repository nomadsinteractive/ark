#include "core/util/uploader_type.h"

#include "core/inf/array.h"
#include "core/impl/uploader/uploader_impl.h"
#include "core/impl/uploader/uploader_array.h"
#include "core/impl/uploader/uploader_repeat.h"
#include "core/impl/uploader/uploader_variable_array.h"
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
    UploaderList(Vector<sp<Uploader>> uploaders)
        : Uploader(0) {
        for(sp<Uploader>& i : uploaders) {
            const size_t size = i->size();
            _uploaders.push_back({_size, std::move(i), true});
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
    Vector<Node> _uploaders;
};

struct WritableSnapshot final : Writable {

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        const auto iter = _records.lower_bound(offset);
        if(iter != _records.begin() && !_records.empty())
            if(const auto previter = std::prev(iter); previter->first + previter->second.size() == offset)
            {
                Vector<uint8_t>& prev = previter->second;
                size_t prevsize = prev.size();
                prev.resize(prevsize + size);
                memcpy(prev.data() + prevsize, buffer, size);
                return size;
            }
        Vector<uint8_t> data(size);
        memcpy(data.data(), buffer, size);
        _records.insert(iter, {offset, std::move(data)});
        return size;
    }

    Map<size_t, Vector<uint8_t>> _records;
};

struct WritableRangeSnapshot final : Writable {

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        const auto iter = _records.lower_bound(offset);
        if(iter != _records.begin() && !_records.empty())
            if(const auto previter = std::prev(iter); previter->first + previter->second == offset)
            {
                size_t& prev = previter->second;
                prev += size;
                return size;
            }
        _records.insert(iter, {offset, size});
        return size;
    }

    Map<size_t, size_t> _records;
};

class UploaderDyed final : public Uploader, public Wrapper<Uploader> {
public:
    UploaderDyed(sp<Uploader> delegate, String message)
        : Uploader(delegate->size()), Wrapper(std::move(delegate)), _message(std::move(message)) {
    }

    bool update(uint64_t timestamp) override
    {
        return _wrapped->update(timestamp);
    }

    void upload(Writable& buf) override
    {
        TRACE(true, _message.c_str());
        _wrapped->upload(buf);
    }

private:
    String _message;
};

sp<UploaderImpl> ensureImpl(const sp<Uploader>& self)
{
    return self.ensureInstance<UploaderImpl>("This object is not a InputImpl instance. Use \"reserve\" method to create an InputImpl instance.");
}

}

sp<Uploader> UploaderType::create(sp<ByteArray> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderArray<uint8_t>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<IntArray> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderArray<int32_t>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Integer> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderOfVariable<int32_t>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Numeric> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderOfVariable<float>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Vec2> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderOfVariable<V2>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Vec3> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderOfVariable<V3>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(sp<Vec4> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderOfVariable<V4>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(Map<size_t, sp<Uploader>> value, size_t size)
{
    return sp<Uploader>::make<UploaderImpl>(std::move(value), size);
}

sp<Uploader> UploaderType::create(Vector<sp<Mat4>> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderVariableArray<M4>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(Vector<sp<Uploader>> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderList>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(Vector<V3> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderArray<V3>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(Vector<V4> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderArray<V4>>(std::move(value)), size);
}

sp<Uploader> UploaderType::create(Vector<uint32_t> value, size_t size)
{
    return reserve(sp<Uploader>::make<UploaderArray<uint32_t>>(std::move(value)), size);
}

Vector<uint8_t> UploaderType::toBytes(Uploader& self)
{
    Vector<uint8_t> bytes(self.size());
    writeTo(self, bytes.data());
    return bytes;
}

Map<size_t, Vector<uint8_t>> UploaderType::record(Uploader& self)
{
    WritableSnapshot writable;
    self.upload(writable);
    return std::move(writable._records);
}

Map<size_t, size_t> UploaderType::recordRanges(Uploader& self)
{
    WritableRangeSnapshot writable;
    self.upload(writable);
    return std::move(writable._records);
}

void UploaderType::writeTo(Uploader& self, void* ptr)
{
    WritableMemory memory(ptr);
    self.upload(memory);
}

sp<Uploader> UploaderType::wrap(sp<Uploader> self)
{
    return sp<Uploader>::make<UploaderWrapper>(std::move(self));
}

sp<Uploader> UploaderType::dye(sp<Uploader> self, String message)
{
    return sp<Uploader>::make<UploaderDyed>(std::move(self), std::move(message));
}

sp<Uploader> UploaderType::makeElementIndexInput(Vector<element_index_t> value)
{
    return sp<Uploader>::make<UploaderArray<element_index_t>>(std::move(value));
}

void UploaderType::reset(const sp<Uploader>& self, sp<Uploader> delegate)
{
    if(const sp<UploaderImpl> impl = self.asInstance<UploaderImpl>())
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
    return sp<Uploader>::make<UploaderRepeat>(std::move(self), length, stride);
}

void UploaderType::put(const sp<Uploader>& self, size_t offset, sp<Uploader> input)
{
    ensureImpl(self)->addInput(offset, std::move(input));
}

void UploaderType::remove(const sp<Uploader>& self, size_t offset)
{
    ensureImpl(self)->removeInput(offset);
}

void UploaderType::markDirty(const sp<Uploader>& self)
{
    ensureImpl(self)->markDirty();
}

sp<UploaderWrapper> UploaderType::ensureWrapper(const sp<Uploader>& self)
{
    return self.ensureInstance<UploaderWrapper>("This Uploader object is not a UploaderWrapper instance");
}

}
