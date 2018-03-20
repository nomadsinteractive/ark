#include "graphics/base/vec2.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/base/variable_wrapper.h"
#include "core/impl/numeric/translate.h"
#include "core/inf/array.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"
#include "core/util/strings.h"

#include "graphics/impl/vv2/vv2_with_transform.h"

namespace ark {

Vec2::Vec2() noexcept
    : _x(sp<NumericWrapper>::make(0.0f)), _y(sp<NumericWrapper>::make(0.0f))
{
}

Vec2::Vec2(float x, float y) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y))
{
}

Vec2::Vec2(const V2& xy) noexcept
    : Vec2(xy.x(), xy.y())
{
}

Vec2::Vec2(const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept
    : _x(sp<NumericWrapper>::make(xProperty)), _y(sp<NumericWrapper>::make(yProperty))
{
}

Vec2::Vec2(float x, float y, const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept
    : _x(sp<NumericWrapper>::make(sp<Translate>::make(xProperty, x))), _y(sp<NumericWrapper>::make(sp<Translate>::make(yProperty, y)))
{
}

Vec2::Vec2(VV2& other) noexcept
    : Vec2(other.val())
{
}

V2 Vec2::val()
{
    return V2(_x->val(), _y->val());
}

Vec2 operator +(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(NumericUtil::add(lvalue._x, rvalue._x), NumericUtil::add(lvalue._y, rvalue._y));
}

Vec2 operator -(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(NumericUtil::sub(lvalue._x, rvalue._x), NumericUtil::sub(lvalue._y, rvalue._y));
}

Vec2 operator *(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(NumericUtil::mul(lvalue._x, rvalue._x), NumericUtil::mul(lvalue._y, rvalue._y));
}

Vec2 operator /(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(NumericUtil::truediv(lvalue._x, rvalue._x), NumericUtil::truediv(lvalue._y, rvalue._y));
}

sp<Vec2> Vec2::negative()
{
    return sp<Vec2>::make(0.0f, 0.0f, NumericUtil::negative(_x), NumericUtil::negative(_y));
}

float Vec2::x() const
{
    return _x->val();
}

float Vec2::y() const
{
    return _y->val();
}

V2 Vec2::xy() const
{
    return V2(_x->val(), _y->val());
}

void Vec2::setX(float x)
{
    _x->set(x);
}

void Vec2::setY(float y)
{
    _y->set(y);
}

void Vec2::setXy(const V2& xy)
{
    _x->set(xy.x());
    _y->set(xy.y());
}

sp<Numeric> Vec2::vx() const
{
    return _x;
}

sp<Numeric> Vec2::vy() const
{
    return _y;
}

void Vec2::setVx(const sp<Numeric>& vx) const
{
    _x->set(vx);
}

void Vec2::setVy(const sp<Numeric>& vy) const
{
    _y->set(vy);
}

void Vec2::assign(const sp<Vec2>& delegate)
{
    if(delegate)
    {
        _x->set(delegate->_x);
        _y->set(delegate->_y);
    }
    else
    {
        _x->set(nullptr);
        _y->set(nullptr);
    }
}

void Vec2::fix()
{
    _x->fix();
    _y->fix();
}

Vec2 Vec2::translate(const Vec2& translation) const
{
    return Vec2(NumericUtil::add(_x, translation._x), NumericUtil::add(_y, translation._y));
}

Vec2 Vec2::translate(float x, float y) const
{
    return Vec2(sp<Translate>::make(_x, x), sp<Translate>::make(_y, y));
}

Vec2 Vec2::translate(ObjectPool& op, float x, float y) const
{
    return Vec2(op.obtain<Translate>(_x, x), op.obtain<Translate>(_y, y));
}

sp<VV2> Vec2::transform(const sp<Transform>& transform, const sp<VV2>& org) const
{
    return sp<VV2WithTransform>::make(sp<Vec2>::make(*this), org, transform);
}

Vec2::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.getBuilder<Numeric>(doc, "x")), _y(parent.getBuilder<Numeric>(doc, "y"))
{
}

sp<Vec2> Vec2::BUILDER::build(const sp<Scope>& args)
{
    return sp<Vec2>::make(_x->build(args), _y->build(args));
}

template<> ARK_API const sp<Vec2> Null::ptr()
{
    return Ark::instance().obtain<Vec2>();
}

Vec2::DICTIONARY::DICTIONARY(BeanFactory& factory, const String str)
{
    BeanUtils::parse<Numeric, Numeric>(factory, str, _x, _y);
}

sp<Vec2> Vec2::DICTIONARY::build(const sp<Scope>& args)
{
    const sp<Numeric> x = _x->build(args);
    return sp<Vec2>::make(_x->build(args), _y ? _y->build(args) : x);
}

Vec2::VV2_DICTIONARY::VV2_DICTIONARY(BeanFactory& parent, const String str)
    : _delegate(parent, str)
{
}

sp<VV2> Vec2::VV2_DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

Vec2::VV2_BUILDER::VV2_BUILDER(BeanFactory& parent, const document& doc)
    : _impl(parent, doc)
{
}

sp<VV2> Vec2::VV2_BUILDER::build(const sp<Scope>& args)
{
    return _impl.build(args);
}

}
