#include "renderer/impl/uploader/uploader_recoder.h"


namespace ark {

namespace {

class WritableImpl : public Writable {
public:
    WritableImpl(size_t size)
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

UploaderRecorder::UploaderRecorder(Uploader& delegate)
    : Uploader(delegate.size())
{
    WritableImpl writable(_size);
    delegate.upload(writable);
    _strips = std::move(writable._strips);
}

void UploaderRecorder::upload(Writable& writable)
{
    for(const auto& [i, j] : _strips)
        writable.write(j->buf(), j->length(), i);
}

std::vector<std::pair<size_t, ByteArray::Borrowed>> UploaderRecorder::toStrips() const
{
    std::vector<std::pair<size_t, ByteArray::Borrowed>> strips;
    for(const auto& [i, j] : _strips)
        strips.push_back(std::make_pair(i, ByteArray::Borrowed(j->buf(), j->length())));
    return strips;
}

}
