#pragma once

#include "core/base/api.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API V4 : public V3 {
public:
    explicit V4(float v = 0);
    V4(float x, float y, float z, float w);
    V4(const V2& xy, float z, float w);
    V4(const V3& xyz, float w);
    V4(const std::initializer_list<float>& values);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(V4);

    bool operator ==(const V4& other) const;
    bool operator !=(const V4& other) const;

    V4& operator +=(const V4& other);
    V4& operator -=(const V4& other);
    V4& operator *=(const V4& other);
    V4& operator /=(const V4& other);

    friend ARK_API V4 operator +(const V4& lvalue, const V4& rvalue);
    friend ARK_API V4 operator -(const V4& lvalue, const V4& rvalue);
    friend ARK_API V4 operator *(const V4& lvalue, const V4& rvalue);
    friend ARK_API V4 operator *(const V4& lvalue, float rvalue);
    friend ARK_API V4 operator *(float lvalue, const V4& rvalue);
    friend ARK_API V4 operator /(const V4& lvalue, const V4& rvalue);
    friend ARK_API V4 operator /(const V4& lvalue, float rvalue);

    float w() const;
    V4 operator -() const;

    V4 floorDiv(const V4& other) const;

    V3 toNonHomogeneous() const;

private:
    float _w;

    friend class Color;
};

}
