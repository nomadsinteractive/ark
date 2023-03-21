#pragma once

#include "core/inf/array.h"
#include "core/inf/uploader.h"
#include "core/inf/writable.h"

namespace ark {

template<typename T> class UploaderArray : public Uploader {
public:
    UploaderArray(sp<Array<T>> vector)
        : Uploader(vector->size()), _vector(std::move(vector)) {
    }
    UploaderArray(std::vector<T> vector)
        : UploaderArray(sp<typename Array<T>::Vector>::make(std::move(vector))) {
    }

    virtual void upload(Writable& buf) override {
        buf.write(_vector->buf(), _vector->size(), 0);
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    sp<Array<T>> _vector;

};

}
