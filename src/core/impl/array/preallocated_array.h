#ifndef ARK_CORE_IMPL_PREALLOCATED_ARRAY_H_
#define ARK_CORE_IMPL_PREALLOCATED_ARRAY_H_

#include "core/forwarding.h"
#include "core/inf/array.h"

namespace ark {

template<typename T> class PreallocatedArray : public Array<T> {
public:
    PreallocatedArray(T* data, uint32_t length)
        : _data(data), _length(length) {
    }

    virtual uint32_t length() override {
        return _length;
    }

    virtual T* array() override {
        return _data;
    }

private:
    T* _data;
    uint32_t _length;
};

}

#endif
