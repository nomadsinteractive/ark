#ifndef ARK_GRAPHICS_BASE_MATRIX_H_
#define ARK_GRAPHICS_BASE_MATRIX_H_

#include "core/base/api.h"

namespace ark {

class Matrix {
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

private:
    template<typename T> const T& matrix() const {
        return *reinterpret_cast<const T*>(_value);
    }

    template<typename T> T& matrix() {
        return *reinterpret_cast<T*>(_value);
    }

private:
    float _value[16];
};

}
#endif
