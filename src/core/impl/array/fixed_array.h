#ifndef ARK_CORE_IMPL_FIXED_ARRAY_H_
#define ARK_CORE_IMPL_FIXED_ARRAY_H_

#include <array>

#include "core/forwarding.h"
#include "core/inf/array.h"

namespace ark {

template<typename T, size_t LEN> class FixedArray : public Array<T> {
public:
    FixedArray() {
    }

    FixedArray(const std::initializer_list<T>& list) {
        std::copy(list.begin(), list.begin() + std::min(LEN, list.size()), _data.begin());
    }

    virtual size_t length() override {
        return _data.size();
    }

    virtual T* buf() override {
        return _data.data();
    }

private:
    std::array<T, LEN> _data;
};

}

#endif
