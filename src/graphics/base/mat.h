#ifndef ARK_GRAPHICS_BASE_MAT_H_
#define ARK_GRAPHICS_BASE_MAT_H_

#include <cstring>

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"
#include "graphics/util/matrix_util.h"

namespace ark {

template<uint32_t S> class Mat {
public:
    Mat() {
        setIndentity();
    }
    template<typename T> Mat(const T& other) {
        DCHECK(sizeof(_value) == sizeof(T), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(T));
        memcpy(_value, &other, sizeof(_value));
        DASSERT(sanityCheck());
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mat);

    static Mat<S> identity() {
        static Mat<S> m;
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

    Mat<S> transpose() const {
        return MatrixUtil::transpose(*this);
    }

    friend ARK_API Mat<S> operator *(const Mat<S>& lvalue, const Mat<S>& rvalue) {
        return MatrixUtil::mul(lvalue, rvalue);
    }

    friend ARK_API V2 operator *(const Mat<S>& lvalue, const V2& rvalue) {
        return MatrixUtil::mul(lvalue, rvalue);
    }

    friend ARK_API V3 operator *(const Mat<S>& lvalue, const V3& rvalue) {
        return MatrixUtil::mul(lvalue, rvalue);
    }

    friend ARK_API V4 operator *(const Mat<S>& lvalue, const V4& rvalue) {
        return MatrixUtil::mul(lvalue, rvalue);
    }

    const float& operator[] (size_t i) const {
        return _value[i];
    }

    float& operator[] (size_t i) {
        return _value[i];
    }

private:
    void setIndentity() {
        memset(_value, 0, sizeof(_value));
        for(uint32_t i = 0; i < S; ++i)
            _value[i * S + i] = 1.0f;
    }

    bool sanityCheck() const {
        for(size_t i = 0; i < S * S; ++i)
            if(std::isnan(_value[i]) || std::isinf(_value[i]))
                return false;
        return true;
    }

private:
    float _value[S * S];
};

}
#endif
