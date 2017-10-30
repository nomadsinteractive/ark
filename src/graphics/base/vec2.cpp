#include "graphics/base/vec2.h"

#include <algorithm>

#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/impl/numeric/scalar.h"
#include "core/impl/numeric/negative.h"
#include "core/impl/numeric/add.h"
#include "core/impl/numeric/divide.h"
#include "core/impl/numeric/multiply.h"
#include "core/impl/numeric/subtract.h"
#include "core/impl/numeric/translate.h"
#include "core/inf/array.h"
#include "core/util/bean_utils.h"
#include "core/util/strings.h"

#include "graphics/base/v2.h"
#include "graphics/impl/vv2/vv2_with_transform.h"

namespace ark {

Vec2::Vec2() noexcept
    : _x(sp<Scalar>::make(0.0f)), _y(sp<Scalar>::make(0.0f))
{
}

Vec2::Vec2(float x, float y) noexcept
    : _x(sp<Scalar>::make(x)), _y(sp<Scalar>::make(y))
{
}

Vec2::Vec2(const V2& xy) noexcept
    : Vec2(xy.x(), xy.y())
{
}

Vec2::Vec2(const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept
    : _x(sp<Scalar>::make(xProperty)), _y(sp<Scalar>::make(yProperty))
{
}

Vec2::Vec2(float x, float y, const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept
    : _x(sp<Scalar>::make(sp<Translate>::make(xProperty, x))), _y(sp<Scalar>::make(sp<Translate>::make(yProperty, y)))
{
}

Vec2::Vec2(const Vec2& other) noexcept
    : _x(other._x), _y(other._y)
{
}

Vec2::Vec2(Vec2&& other) noexcept
    : _x(std::move(other._x)), _y(std::move(other._y))
{
}

Vec2::Vec2(VV2& other) noexcept
    : Vec2(other.val())
{
}

const Vec2& Vec2::operator =(const Vec2& other)
{
    _x = other._x;
    _y = other._y;
    return *this;
}

const Vec2& Vec2::operator =(Vec2&& other)
{
    _x = std::move(other._x);
    _y = std::move(other._y);
    return *this;
}

V2 Vec2::val()
{
    return V2(_x->val(), _y->val());
}

Vec2 operator +(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(sp<Add>::make(lvalue._x, rvalue._x).cast<Numeric>(), sp<Add>::make(lvalue._y, rvalue._y).cast<Numeric>());
}

Vec2 operator -(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(sp<Subtract>::make(lvalue._x, rvalue._x).cast<Numeric>(), sp<Subtract>::make(lvalue._y, rvalue._y).cast<Numeric>());
}

Vec2 operator *(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(sp<Multiply>::make(lvalue._x, rvalue._x).cast<Numeric>(), sp<Multiply>::make(lvalue._y, rvalue._y).cast<Numeric>());
}

Vec2 operator /(const Vec2& lvalue, const Vec2& rvalue)
{
    return Vec2(sp<Divide>::make(lvalue._x, rvalue._x).cast<Numeric>(), sp<Divide>::make(lvalue._y, rvalue._y).cast<Numeric>());
}

sp<Vec2> Vec2::negative()
{
    return sp<Vec2>::make(0.0f, 0.0f, sp<Negative>::make(_x), sp<Negative>::make(_y));
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

void Vec2::setXY(float x, float y)
{
    _x->set(x);
    _y->set(y);
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
    _x->setDelegate(Null::toSafe<Numeric>(vx));
}

void Vec2::setVy(const sp<Numeric>& vy) const
{
    _y->setDelegate(Null::toSafe<Numeric>(vy));
}

void Vec2::assign(const sp<Vec2>& delegate)
{
    if(delegate)
    {
        _x->assign(delegate->_x);
        _y->assign(delegate->_y);
    }
    else
    {
        _x->assign(nullptr);
        _y->assign(nullptr);
    }
}

void Vec2::fix()
{
    _x->fix();
    _y->fix();
}

Vec2 Vec2::translate(const Vec2& translation) const
{
    return Vec2(sp<Add>::make(_x, translation._x), sp<Add>::make(_y, translation._y));
}

Vec2 Vec2::translate(float x, float y) const
{
    return Vec2(sp<Translate>::make(_x, x), sp<Translate>::make(_y, y));
}

Vec2 Vec2::translate(ObjectPool<Numeric>& op, float x, float y) const
{
    return Vec2(op.allocate<Translate>(_x, x), op.allocate<Translate>(_y, y));
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

template<> ARK_API const sp<Vec2>& Null::ptr()
{
    static sp<Vec2> instance = sp<Vec2>::make();
    return instance;
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
