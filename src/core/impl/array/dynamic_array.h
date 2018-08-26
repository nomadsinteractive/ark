#ifndef ARK_CORE_IMPL_ARRAY_DYNAMIC_ARRAY_H_
#define ARK_CORE_IMPL_ARRAY_DYNAMIC_ARRAY_H_

#include "core/inf/array.h"

namespace ark {

template<typename T> class DynamicArray : public Array<T> {
public:
    DynamicArray(size_t length)
        : _data(new T[length]), _length(length) {
    }

    DynamicArray(const std::initializer_list<T>& list)
        : _data(list) {
    }

    DynamicArray(const DynamicArray<T>& other) = delete;

    ~DynamicArray() {
        delete[] _data;
    }

    virtual size_t length() override {
        return _length;
    }

    virtual T* buf() override {
        return _data;
    }

private:
    T* _data;
    size_t _length;
};

}

#endif
