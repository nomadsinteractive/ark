#ifndef ARK_GRAPHICS_BASE_V3_H_
#define ARK_GRAPHICS_BASE_V3_H_

#include "core/base/api.h"

#include "graphics/base/v2.h"

namespace ark {

class ARK_API V3 : public V2 {
public:
    V3();
    V3(float x, float y, float z);
    V3(const V2& other);
    V3(const std::initializer_list<float>& values);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(V3);

    bool operator ==(const V3& other) const;
    bool operator !=(const V3& other) const;

    V3& operator +=(const V3& other);
    V3& operator -=(const V3& other);
    V3& operator *=(const V3& other);
    V3& operator /=(const V3& other);

    friend V3 operator +(const V3& lvalue, const V3& rvalue);
    friend V3 operator -(const V3& lvalue, const V3& rvalue);
    friend V3 operator *(const V3& lvalue, const V3& rvalue);
    friend V3 operator *(const V3& lvalue, float rvalue);
    friend V3 operator *(float lvalue, const V3& rvalue);
    friend V3 operator /(const V3& lvalue, const V3& rvalue);
    friend V3 operator /(const V3& lvalue, float rvalue);

    float z() const;

    V3 operator -() const;

    float dot(const V3& other) const;
    V3 cross(const V3& other) const;

protected:
    float _z;

    friend class V4;
    friend class Color;
};

}

#endif
