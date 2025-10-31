#pragma once

#include "core/inf/array.h"
#include "core/inf/uploader.h"
#include "core/inf/writable.h"

namespace ark {

template<typename T> class UploaderArray final : public Uploader {
public:
    UploaderArray(sp<Array<T>> vector)
        : Uploader(vector->size()), _vector(std::move(vector)) {
    }
    UploaderArray(Vector<T> vector)
        : UploaderArray(sp<typename Array<T>::Vector>::make(std::move(vector))) {
    }

    void upload(Writable& buf) override {
        buf.write(_vector->buf(), _vector->size(), 0);
    }

    bool update(uint32_t /*tick*/) override {
        return false;
    }

private:
    sp<Array<T>> _vector;
};

}
