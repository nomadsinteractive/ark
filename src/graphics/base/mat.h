#ifndef ARK_GRAPHICS_BASE_MAT_H_
#define ARK_GRAPHICS_BASE_MAT_H_

#include <cstring>

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

template<uint32_t S> class Mat {
public:
    Mat() = default;
    template<typename T> Mat(const T& other) {
        DCHECK(sizeof(_value) == sizeof(T), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(T));
        memcpy(_value, &other, sizeof(_value));
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat);

    static Mat<S> identity() {
        Mat<S> m;
        memset(&m, 0, sizeof(m));
        for(uint32_t i = 0; i < S; ++i)
            m._value[i * S + i] = 1.0f;
        return m;
    }

    float* value() {
        return _value;
    }

    const float* value() const {
        return _value;
    }

    template<typename T> const T& mat() const {
        DCHECK(sizeof(_value) == sizeof(T), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(T));
        return *reinterpret_cast<const T*>(_value);
    }

    template<typename T> T& mat() {
        DCHECK(sizeof(_value) == sizeof(T), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(T));
        return *reinterpret_cast<T*>(_value);
    }

private:
    float _value[S * S];
};

}
#endif
