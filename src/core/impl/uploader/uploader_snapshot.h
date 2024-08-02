#pragma once

#include <vector>

#include "core/inf/uploader.h"

namespace ark {

class UploaderSnapshot final : public Uploader {
public:
    UploaderSnapshot(Uploader& delegate);

    void upload(Writable& writable) override;
    bool update(uint64_t timestamp) override;

private:
    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;
};

}
