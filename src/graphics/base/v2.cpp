#include "graphics/base/v2.h"

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

namespace ark {

V2::V2()
    : _x(0), _y(0)
{
}

V2::V2(float x, float y)
    : _x(x), _y(y)
{
}

V2::V2(float x, float y, float /*z*/)
    : _x(x), _y(y)
{
}

V2::V2(const V3& other)
    : _x(other.x()), _y(other.y())
{
}

V2 V2::identity()
{
    return V2(1.0f, 1.0f);
}

bool V2::operator ==(const V2& other) const
{
    return _x == other._x && _y == other._y;
}

bool V2::operator !=(const V2& other) const
{
    return _x != other._x || _y != other._y;
}

V2& V2::operator +=(const V2& other)
{
    _x += other._x;
    _y += other._y;
    return *this;
}

V2& V2::operator *=(const V2& other)
{
    _x *= other._x;
    _y *= other._y;
    return *this;
}

V2 V2::operator -() const
{
    return V2(-_x, -_y);
}

V2 operator +(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue.x() + rvalue.x(), lvalue.y() + rvalue.y());
}

V2 operator -(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue.x() - rvalue.x(), lvalue.y() - rvalue.y());
}

V2 operator *(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue.x() * rvalue.x(), lvalue.y() * rvalue.y());
}

V2 operator /(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue.x() / rvalue.x(), lvalue.y() / rvalue.y());
}

float V2::x() const
{
    return _x;
}

float V2::y() const
{
    return _y;
}

float V2::z() const
{
    return 0;
}

float V2::dot(const V2& other) const
{
    return _x * other._x + _y * other._y;
}

const float& V2::operator[](int32_t p) const
{
    DCHECK(p >=0, "Index out bounds: %d", p);
    return reinterpret_cast<const float*>(this)[p];
}

float& V2::operator[](int32_t p)
{
    DCHECK(p >=0, "Index out bounds: %d", p);
    return reinterpret_cast<float*>(this)[p];
}

}
