#include "graphics/impl/vec/vec2_impl.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"
#include "core/util/strings.h"

#include "graphics/impl/vec/vv2_with_transform.h"

namespace ark {

Vec2Impl::Vec2Impl() noexcept
    : _x(sp<NumericWrapper>::make(0.0f)), _y(sp<NumericWrapper>::make(0.0f))
{
}

Vec2Impl::Vec2Impl(float x, float y) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y))
{
}

Vec2Impl::Vec2Impl(const V2& xy) noexcept
    : Vec2Impl(xy.x(), xy.y())
{
}

Vec2Impl::Vec2Impl(const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept
    : _x(sp<NumericWrapper>::make(xProperty)), _y(sp<NumericWrapper>::make(yProperty))
{
}

Vec2Impl::Vec2Impl(Vec2& other) noexcept
    : Vec2Impl(other.val())
{
}

V2 Vec2Impl::val()
{
    return V2(_x->val(), _y->val());
}

const sp<NumericWrapper>& Vec2Impl::x() const
{
    return _x;
}

const sp<NumericWrapper>& Vec2Impl::y() const
{
    return _y;
}

void Vec2Impl::set(const V2& val)
{
    _x->set(val.x());
    _y->set(val.y());
}

void Vec2Impl::fix()
{
    _x->fix();
    _y->fix();
}

Vec2Impl::VV2_DICTIONARY::VV2_DICTIONARY(BeanFactory& factory, const String str)
{
    BeanUtils::parse<Numeric, Numeric>(factory, str, _x, _y);
}

sp<Vec2> Vec2Impl::VV2_DICTIONARY::build(const sp<Scope>& args)
{
    const sp<Numeric> x = _x->build(args);
    return sp<Vec2Impl>::make(_x->build(args), _y ? _y->build(args) : x);
}

Vec2Impl::VV2_BUILDER::VV2_BUILDER(BeanFactory& factory, const document& doc)
    : _x(factory.getBuilder<Numeric>(doc, "x")), _y(factory.getBuilder<Numeric>(doc, "y"))
{
}

sp<Vec2> Vec2Impl::VV2_BUILDER::build(const sp<Scope>& args)
{
    return sp<Vec2Impl>::make(_x->build(args), _y->build(args));
}

}