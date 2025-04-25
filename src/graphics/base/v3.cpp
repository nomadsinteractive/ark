#include "graphics/base/v3.h"

#include "core/ark.h"
#include "core/util/math.h"

namespace ark {

V3::V3(const float v)
    : V2(v), _z(v)
{
}

V3::V3(const float x, const float y, const float z)
    : V2(x, y), _z(z)
{
}

V3::V3(const V2& other, const float z)
    : V2(other._x, other._y), _z(z)
{
}

V3::V3(const std::initializer_list<float>& values)
{
    initialize(values, 3);
}

bool V3::operator ==(const V3& other) const
{
    return _x == other._x && _y == other._y && _z == other._z;
}

bool V3::operator !=(const V3& other) const
{
    return _x != other._x || _y != other._y || _z != other._z;
}

V3& V3::operator +=(const V3& other)
{
    _x += other._x;
    _y += other._y;
    _z += other._z;
    return *this;
}

V3& V3::operator -=(const V3& other)
{
    _x -= other._x;
    _y -= other._y;
    _z -= other._z;
    return *this;
}

V3& V3::operator *=(const V3& other)
{
    _x *= other._x;
    _y *= other._y;
    _z *= other._z;
    return *this;
}

V3& V3::operator /=(const V3& other)
{
    _x /= other._x;
    _y /= other._y;
    _z /= other._z;
    return *this;
}

V3 operator +(const V3& lvalue, const V3& rvalue)
{
    return {lvalue._x + rvalue._x, lvalue._y + rvalue._y, lvalue._z + rvalue._z};
}

V3 operator -(const V3& lvalue, const V3& rvalue)
{
    return {lvalue._x - rvalue._x, lvalue._y - rvalue._y, lvalue._z - rvalue._z};
}

V3 operator *(const V3& lvalue, const V3& rvalue)
{
    return {lvalue._x * rvalue._x, lvalue._y * rvalue._y, lvalue._z * rvalue._z};
}

V3 operator *(const V3& lvalue, const float rvalue)
{
    return {lvalue._x * rvalue, lvalue._y * rvalue, lvalue._z * rvalue};
}

V3 operator *(const float lvalue, const V3& rvalue)
{
    return {rvalue._x * lvalue, rvalue._y * lvalue, rvalue._z * lvalue};
}

V3 operator /(const V3& lvalue, const V3& rvalue)
{
    return {lvalue._x / rvalue._x, lvalue._y / rvalue._y, lvalue._z / rvalue._z};
}

V3 operator /(const V3& lvalue, float rvalue)
{
    return {lvalue._x / rvalue, lvalue._y / rvalue, lvalue._z / rvalue};
}

float V3::z() const
{
    return _z;
}

V3 V3::operator -() const
{
    return {-_x, -_y, -_z};
}

float V3::dot(const V3& other) const
{
    return _x * other._x + _y * other._y + _z * other._z;
}

float V3::hypot() const
{
    return Math::sqrt(_x * _x + _y * _y + _z * _z);
}

V4 V3::extend(const float w) const
{
    return {_x, _y, _z, w};
}

V3 V3::floorDiv(const V3& other) const
{
    return {Math::floorDiv(_x, other._x), Math::floorDiv(_y, other._y), Math::floorDiv(_z, other._z)};
}

V3 V3::normalize() const
{
    const float length = std::max(hypot(), MIN_NORMALIZE_LENGTH);
    return {_x / length, _y / length, _z / length};
}

V3 V3::cross(const V3& other) const
{
    return {_y * other._z - other._y * _z, _z * other._x - other._z * _x, _x * other._y - other._x * _y};
}

}
