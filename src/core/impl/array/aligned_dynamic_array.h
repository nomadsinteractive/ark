#ifndef ARK_CORE_IMPL_ARRAY_ALIGNED_DYNAMIC_ARRAY_H_
#define ARK_CORE_IMPL_ARRAY_ALIGNED_DYNAMIC_ARRAY_H_

#include "core/inf/array.h"

namespace ark {

template<typename T, typename ALIGNMENT = T> class AlignedDynamicArray : public Array<T> {
public:
    AlignedDynamicArray(size_t length)
        : _data(new ALIGNMENT[sizeof(T) * length / sizeof(ALIGNMENT)]), _ptr(reinterpret_cast<T*>(_data)), _length(length) {
        DCHECK((sizeof(T) * length) % sizeof(ALIGNMENT) == 0, "length = %d, alignment = %d", length, sizeof(ALIGNMENT));
    }
    AlignedDynamicArray(const AlignedDynamicArray& other) = delete;
    ~AlignedDynamicArray() {
        delete[] _data;
    }

    virtual size_t length() override {
        return _length;
    }

    virtual T* buf() override {
        return _ptr;
    }

private:
    ALIGNMENT* _data;
    T* _ptr;
    size_t _length;
};

}

#endif
