#ifndef ARK_GRAPHICS_BASE_MATRIX_H_
#define ARK_GRAPHICS_BASE_MATRIX_H_

#include <string.h>

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Matrix {
public:
    Matrix();
    Matrix(const Matrix& other);

    const Matrix operator *(const Matrix& right) const;

    const Matrix& operator =(const Matrix& other);

    void setIdentity();
    void translate(float x, float y, float z);
    void rotate(float degree, float x, float y, float z);
    void scale(float x, float y, float z);

    float* value();
    const float* value() const;
    void map(const float x, const float y, const float z, float& transformedX, float& transformedY, float& transformedZ) const;

    static Matrix ortho(float left, float right, float top, float bottom, float near, float far);
    static Matrix lookAt(const V3& position, const V3& target, const V3& up);
    static Matrix perspective(float fov, float aspect, float near, float far);

private:
    template<typename T> const T& matrix() const {
        return *reinterpret_cast<const T*>(_value);
    }

    template<typename T> T& matrix() {
        return *reinterpret_cast<T*>(_value);
    }

    template<typename T> Matrix(const T& other) {
        DCHECK(sizeof(_value) == sizeof(T), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(T));
        memcpy(_value, &other, sizeof(_value));
    }

private:
    float _value[16];
};

}
#endif
