#pragma once

#include "core/inf/uploader.h"

namespace ark {

class UploaderSnapshot final : public Uploader {
public:
    UploaderSnapshot(Uploader& delegate);

    void upload(Writable& writable) override;
    bool update(uint64_t timestamp) override;

private:
    Map<size_t, Vector<uint8_t>> _strips;
};

}
