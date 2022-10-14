#include "renderer/impl/uploader/uploader_recoder.h"


namespace ark {

namespace {

class WritableImpl : public Writable {
public:

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        sp<ByteArray> strip = sp<ByteArray::Allocated>::make(size);
        memcpy(strip->buf(), buffer, size);
        _strips.push_back(std::make_pair<size_t, sp<ByteArray>>(static_cast<size_t>(offset), std::move(strip)));
        return size;
    }

    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;

};

}

UploaderRecorder::UploaderRecorder(Uploader& delegate)
    : Uploader(delegate.size())
{
    WritableImpl writable;
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
