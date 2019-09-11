#ifndef ARK_GRAPHICS_BASE_V2_H_
#define ARK_GRAPHICS_BASE_V2_H_

#include <initializer_list>

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API V2 {
public:
    V2(float v = 0);
    V2(float x, float y);
    V2(float x, float y, float z);
    V2(const V3& other);
    V2(const std::initializer_list<float>& values);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(V2);

    bool operator ==(const V2& other) const;
    bool operator !=(const V2& other) const;

    V2& operator +=(const V2& other);
    V2& operator -=(const V2& other);
    V2& operator *=(const V2& other);
    V2& operator /=(const V2& other);

    friend V2 operator +(const V2& lvalue, const V2& rvalue);
    friend V2 operator -(const V2& lvalue, const V2& rvalue);
    friend V2 operator *(const V2& lvalue, const V2& rvalue);
    friend V2 operator *(const V2& lvalue, float rvalue);
    friend V2 operator *(float lvalue, const V2& rvalue);
    friend V2 operator /(const V2& lvalue, const V2& rvalue);
    friend V2 operator /(const V2& lvalue, float rvalue);

    V2 operator -() const;

    float x() const;
    float y() const;
    float z() const;

    float dot(const V2& other) const;
    float length() const;
    V2 normalize() const;

    const float& operator[](int32_t p) const;
    float& operator[](int32_t p);

protected:
    void initialize(const std::initializer_list<float>& values, size_t len);

    static const float MIN_NORMALIZE_LENGTH;

protected:
    float _x;
    float _y;

    friend class V3;
    friend class V4;
    friend class Color;
};

}

#endif
