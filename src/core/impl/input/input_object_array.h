#ifndef ARK_CORE_IMPL_INPUT_INPUT_OBJECT_ARRAY_H_
#define ARK_CORE_IMPL_INPUT_INPUT_OBJECT_ARRAY_H_

#include "core/inf/array.h"
#include "core/inf/input.h"
#include "core/inf/writable.h"


namespace ark {

template<typename T> class InputObjectArray : public Input {
public:
    InputObjectArray(sp<Array<T>> vector)
        : Input(vector->size()), _vector(std::move(vector)) {
    }
    InputObjectArray(std::vector<T> vector)
        : InputObjectArray(sp<Array<T>::Vector>::make(std::move(vector))) {
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

#endif
