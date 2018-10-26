#include "graphics/base/v3.h"

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

V3::V3()
    : V2(), _z(0)
{
}

V3::V3(float x, float y, float z)
    : V2(x, y), _z(z)
{
}

V3::V3(const V2& other)
    : V2(other._x, other._y), _z(0)
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

V3 operator +(const V3& lvalue, const V3& rvalue)
{
    return V3(lvalue._x + rvalue._x, lvalue._y + rvalue._y, lvalue._z + rvalue._z);
}

V3 operator -(const V3& lvalue, const V3& rvalue)
{
    return V3(lvalue._x - rvalue._x, lvalue._y - rvalue._y, lvalue._z - rvalue._z);
}

V3 operator *(const V3& lvalue, const V3& rvalue)
{
    return V3(lvalue._x * rvalue._x, lvalue._y * rvalue._y, lvalue._z * rvalue._z);
}

V3 operator *(const V3& lvalue, float rvalue)
{
    return V3(lvalue._x * rvalue, lvalue._y * rvalue, lvalue._z * rvalue);
}

V3 operator /(const V3& lvalue, const V3& rvalue)
{
    return V3(lvalue._x / rvalue._x, lvalue._y / rvalue._y, lvalue._z / rvalue._z);
}

V3 operator /(const V3& lvalue, float rvalue)
{
    return V3(lvalue._x / rvalue, lvalue._y / rvalue, lvalue._z / rvalue);
}

float V3::z() const
{
    return _z;
}

V3 V3::operator -() const
{
    return V3(-_x, -_y, -_z);
}

float V3::dot(const V3& other) const
{
    return _x * other._x + _y * other._y + _z * other._z;
}

V3 V3::cross(const V3& other) const
{
    return V3(_y * other._z - other._y * _z, _z * other._x - other._z * _x, _x * other._y - other._x * _y);
}

}
