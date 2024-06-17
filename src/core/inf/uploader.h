#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/updatable.h"

namespace ark {

class ARK_API Uploader : public Updatable {
public:
    Uploader(size_t size)
        : _size(size) {
    }
    ~Uploader() override = default;

    virtual void upload(Writable& buf) = 0;

    size_t size() const {
        return _size;
    }

protected:
    size_t _size;
};

}
