#include "graphics/base/v2.h"

#include "core/base/range.h"
#include "core/base/slice.h"
#include "core/util/string_convert.h"
#include "core/util/math.h"
#include "core/util/strings.h"

namespace ark {

const float V2::MIN_NORMALIZE_LENGTH = 0.000000001f;

V2::V2(float v)
    : V2(v, v)
{
}

V2::V2(float x, float y)
    : _x(x), _y(y)
{
    DASSERT(!(std::isinf(_x) || std::isinf(_y)));
}

V2::V2(const std::initializer_list<float>& values)
{
    initialize(values, 2);
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

V2& V2::operator -=(const V2& other)
{
    _x -= other._x;
    _y -= other._y;
    return *this;
}

V2& V2::operator *=(const V2& other)
{
    _x *= other._x;
    _y *= other._y;
    return *this;
}

V2& V2::operator /=(const V2& other)
{
    _x /= other._x;
    _y /= other._y;
    return *this;
}

bool V2::operator <(const V2& other) const
{
    return _x < other._x ? true : (_x == other._x ? _y < other._y : false);
}

V2 V2::operator -() const
{
    return V2(-_x, -_y);
}

V2 operator +(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue._x + rvalue._x, lvalue._y + rvalue._y);
}

V2 operator -(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue._x - rvalue._x, lvalue._y - rvalue._y);
}

V2 operator *(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue._x * rvalue._x, lvalue._y * rvalue._y);
}

V2 operator *(const V2& lvalue, float rvalue)
{
    return V2(lvalue._x * rvalue, lvalue._y * rvalue);
}

V2 operator *(float lvalue, const V2& rvalue)
{
    return V2(rvalue._x * lvalue, rvalue._y * lvalue);
}

V2 operator /(const V2& lvalue, const V2& rvalue)
{
    return V2(lvalue._x / rvalue._x, lvalue._y / rvalue._y);
}

V2 operator /(const V2& lvalue, float rvalue)
{
    return V2(lvalue._x / rvalue, lvalue._y / rvalue);
}

float V2::x() const
{
    return _x;
}

float V2::y() const
{
    return _y;
}

float V2::dot(const V2& other) const
{
    return _x * other._x + _y * other._y;
}

float V2::hypot() const
{
    return Math::sqrt(_x * _x + _y * _y);
}

V3 V2::extend(float z) const
{
    return V3(_x, _y, z);
}

V4 V2::extend(const V2& zw) const
{
    return V4(_x, _y, zw.x(), zw.y());
}

V2 V2::floorDiv(const V2& other) const
{
    return V2(Math::floorDiv(_x, other._x), Math::floorDiv(_y, other._y));
}

V2 V2::normalize() const
{
    float length = std::max(hypot(), MIN_NORMALIZE_LENGTH);
    return V2(_x / length, _y / length);
}

const float* V2::value() const
{
    return &_x;
}

float* V2::value()
{
    return &_x;
}

const float& V2::operator[](size_t idx) const
{
    return reinterpret_cast<const float*>(this)[idx];
}

Range<float> V2::subscribe(const Slice& slice, size_t length)
{
    const Slice adjusted = slice.adjustIndices(length);
    return Range<float>(&_x + adjusted.begin(), &_x + adjusted.end(), slice.step());
}

float& V2::operator[](size_t idx)
{
    return reinterpret_cast<float*>(this)[idx];
}

void V2::initialize(const std::initializer_list<float>& values, size_t len)
{
    DASSERT(values.size() <= len);

    memset(this, 0, sizeof(float) * len);

    int32_t idx = 0;
    for(float i : values)
        (*this)[idx++] = i;
}

template<> ARK_API V2 StringConvert::eval<V2>(const String& str)
{
    std::vector<String> sst = str.split(',');
    DCHECK(sst.size() == 2, "Illegal size: %s", str.c_str());
    return V2(Strings::eval<float>(sst.at(0)), Strings::eval<float>(sst.at(1)));
}

}
