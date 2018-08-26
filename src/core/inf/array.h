#ifndef ARK_CORE_INF_ARRAY_H_
#define ARK_CORE_INF_ARRAY_H_

#include <stdint.h>

namespace ark {

template<typename T> class Array {
public:
    virtual ~Array() = default;

    virtual size_t length() = 0;
    virtual T* buf() = 0;

    size_t size() {
        return sizeof(T) * this->length();
    }
};

}

#endif
