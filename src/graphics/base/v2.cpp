#include "graphics/base/v2.h"

#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

V2::V2()
    : _x(0), _y(0)
{
}

V2::V2(float x, float y)
    : _x(x), _y(y)
{
}

V2::V2(const V2& other)
    : _x(other._x), _y(other._y)
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

const V2& V2::operator =(const V2& other)
{
    _x = other._x;
    _y = other._y;
    return *this;
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

template<> ARK_API const sp<VV2>& Null::ptr()
{
    static sp<VV2> instance = sp<VV2::Const>::make(V2());
    return instance;
}


}
