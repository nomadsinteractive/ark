#pragma once

#include <vector>

#include "core/inf/uploader.h"

namespace ark {

class InputSnapshot : public Uploader {
public:
    InputSnapshot(Uploader& delegate);

    virtual void upload(Writable& writable) override;
    virtual bool update(uint64_t timestamp) override;

private:
    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;

};

}
