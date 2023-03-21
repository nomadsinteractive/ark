#include "core/impl/uploader/uploader_snapshot.h"

#include "core/inf/array.h"
#include "core/inf/writable.h"
#include "core/types/shared_ptr.h"

namespace ark {

namespace {

class WritableSnapshot : public Writable {
public:
    WritableSnapshot(size_t size)
        : _size(size) {
    }

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        CHECK(offset + size <= _size, "Buffer overflow, buffer size: %zd, writing offset: %d, writing size: %d", _size, offset, size);
        sp<ByteArray> strip = sp<ByteArray::Allocated>::make(size);
        memcpy(strip->buf(), buffer, size);
        _strips.push_back(std::make_pair<size_t, sp<ByteArray>>(static_cast<size_t>(offset), std::move(strip)));
        return size;
    }

    size_t _size;
    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;

};

}

InputSnapshot::InputSnapshot(Uploader& delegate)
    : Uploader(delegate.size())
{
    WritableSnapshot writable(_size);
    delegate.upload(writable);
    _strips = std::move(writable._strips);
}

void InputSnapshot::upload(Writable& writable)
{
    for(const auto& [i, j] : _strips)
        writable.write(j->buf(), j->length(), i);
}

bool InputSnapshot::update(uint64_t /*timestamp*/)
{
    return false;
}

}
