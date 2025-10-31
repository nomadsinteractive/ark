#pragma once

#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

class UploaderRepeat final : public Uploader {
public:
    UploaderRepeat(sp<Uploader> delegate, size_t length, size_t stride = 0);

    bool update(uint32_t tick) override;
    void upload(Writable& writable) override;

private:
    sp<Uploader> _delegate;
    size_t _length;
    size_t _stride;
};

}
