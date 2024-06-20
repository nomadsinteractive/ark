#pragma once

#include <initializer_list>

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API V2 {
public:
    explicit V2(float v = 0);
    V2(float x, float y);
    V2(const std::initializer_list<float>& values);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(V2);

    bool operator ==(const V2& other) const;
    bool operator !=(const V2& other) const;

    V2& operator +=(const V2& other);
    V2& operator -=(const V2& other);
    V2& operator *=(const V2& other);
    V2& operator /=(const V2& other);

    bool operator <(const V2& other) const;

    friend ARK_API V2 operator +(const V2& lvalue, const V2& rvalue);
    friend ARK_API V2 operator -(const V2& lvalue, const V2& rvalue);
    friend ARK_API V2 operator *(const V2& lvalue, const V2& rvalue);
    friend ARK_API V2 operator *(const V2& lvalue, float rvalue);
    friend ARK_API V2 operator *(float lvalue, const V2& rvalue);
    friend ARK_API V2 operator /(const V2& lvalue, const V2& rvalue);
    friend ARK_API V2 operator /(const V2& lvalue, float rvalue);

    V2 operator -() const;

    float x() const;
    float y() const;

    float dot(const V2& other) const;
    float hypot() const;

    V3 extend(float z) const;
    V4 extend(const V2& zw) const;

    V2 floorDiv(const V2& other) const;
    V2 normalize() const;

    const float* value() const;
    float* value();

    const float& operator[](size_t idx) const;
    float& operator[](size_t idx);

    Range<float> subscribe(const Slice& slice, size_t length);

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
