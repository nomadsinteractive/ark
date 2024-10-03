#include "graphics/base/v4.h"

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/types/null.h"
#include "core/util/string_convert.h"
#include "core/util/strings.h"
#include "core/util/math.h"

#include "graphics/base/color.h"

namespace ark {

V4::V4(float v)
    : V3(v), _w(v)
{
}

V4::V4(float x, float y, float z, float w)
    : V3(x, y, z), _w(w)
{
}

V4::V4(const V2& xy, float z, float w)
    : V3(xy.x(), xy.y(), z), _w(w)
{
}

V4::V4(const V3& xyz, float w)
    : V3(xyz), _w(w)
{
}

V4::V4(const std::initializer_list<float>& values)
{
    initialize(values, 4);
}

bool V4::operator ==(const V4& other) const
{
    return _x == other._x && _y == other._y && _z == other._z && _w == other._w;
}

bool V4::operator !=(const V4& other) const
{
    return _x != other._x || _y != other._y || _z != other._z || _w != other._w;
}

V4& V4::operator +=(const V4& other)
{
    _x += other._x;
    _y += other._y;
    _z += other._z;
    _w += other._w;
    return *this;
}

V4& V4::operator -=(const V4& other)
{
    _x -= other._x;
    _y -= other._y;
    _z -= other._z;
    _w -= other._w;
    return *this;
}

V4& V4::operator *=(const V4& other)
{
    _x *= other._x;
    _y *= other._y;
    _z *= other._z;
    _w *= other._w;
    return *this;
}

V4& V4::operator /=(const V4& other)
{
    _x /= other._x;
    _y /= other._y;
    _z /= other._z;
    _w /= other._w;
    return *this;
}

V4 operator +(const V4& lvalue, const V4& rvalue)
{
    return V4(lvalue._x + rvalue._x, lvalue._y + rvalue._y, lvalue._z + rvalue._z, lvalue._w + rvalue._w);
}

V4 operator -(const V4& lvalue, const V4& rvalue)
{
    return V4(lvalue._x - rvalue._x, lvalue._y - rvalue._y, lvalue._z - rvalue._z, lvalue._w - rvalue._w);
}

V4 operator *(const V4& lvalue, const V4& rvalue)
{
    return V4(lvalue._x * rvalue._x, lvalue._y * rvalue._y, lvalue._z * rvalue._z, lvalue._w * rvalue._w);
}

V4 operator *(const V4& lvalue, float rvalue)
{
    return V4(lvalue._x * rvalue, lvalue._y * rvalue, lvalue._z * rvalue, lvalue._w * rvalue);
}

V4 operator *(float a1, const V4& a2)
{
    return V4(a2._x * a1, a2._y * a1, a2._z * a1, a2._w * a1);
}

V4 operator /(const V4& lvalue, const V4& rvalue)
{
    return V4(lvalue._x / rvalue._x, lvalue._y / rvalue._y, lvalue._z / rvalue._z, lvalue._w / rvalue._w);
}

V4 operator /(const V4& lvalue, float rvalue)
{
    return V4(lvalue._x / rvalue, lvalue._y / rvalue, lvalue._z / rvalue, lvalue._w / rvalue);
}

float V4::w() const
{
    return _w;
}

V4 V4::operator -() const
{
    return V4(-_x, -_y, -_z, -_w);
}

float V4::dot(const V4& other) const
{
    return _x * other._x + _y * other._y + _z * other._z + _w * other._w;
}

float V4::hypot() const
{
    return Math::sqrt(_x * _x + _y * _y + _z * _z + _w * _w);
}

V4 V4::normalize() const
{
    const float length = std::max(hypot(), MIN_NORMALIZE_LENGTH);
    return V4(_x / length, _y / length, _z / length, _w / length);
}

V4 V4::floorDiv(const V4& other) const
{
    return V4(Math::floorDiv(_x, other._x), Math::floorDiv(_y, other._y), Math::floorDiv(_z, other._z), Math::floorDiv(_w, other._w));
}

V3 V4::toNonHomogeneous() const
{
    return (_w < 0 ? -V3(_x, _y, _z) : V3(_x, _y, _z)) / std::max(MIN_NORMALIZE_LENGTH, std::abs(_w));
}

}
