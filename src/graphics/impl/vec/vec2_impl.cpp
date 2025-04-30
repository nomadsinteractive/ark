#include "graphics/impl/vec/vec2_impl.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

namespace ark {

Vec2Impl::Vec2Impl() noexcept
    : Vec2Impl(0, 0)
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

Vec2Impl::Vec2Impl(sp<Numeric> v) noexcept
    : _x(sp<NumericWrapper>::make(v)), _y(sp<NumericWrapper>::make(std::move(v)))
{
}

Vec2Impl::Vec2Impl(sp<Numeric> x, sp<Numeric> y) noexcept
    : _x(sp<NumericWrapper>::make(std::move(x))), _y(sp<NumericWrapper>::make(std::move(y)))
{
}

Vec2Impl::Vec2Impl(Vec2& other) noexcept
    : Vec2Impl(other.val())
{
}

V2 Vec2Impl::val()
{
    return {_x->val(), _y->val()};
}

bool Vec2Impl::update(const uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, _x, _y);
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

Vec2Impl::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
{
    factory.expand(expr, _x, _y);
}

sp<Vec2> Vec2Impl::DICTIONARY::build(const Scope& args)
{
    const sp<Numeric> x = _x->build(args);
    const sp<Numeric> y = _y->build(args);
    return sp<Vec2>::make<Vec2Impl>(x, y ? y : x);
}

Vec2Impl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _x(factory.ensureBuilder<Numeric>(manifest, "x")), _y(factory.getBuilder<Numeric>(manifest, "y"))
{
}

sp<Vec2> Vec2Impl::BUILDER::build(const Scope& args)
{
    return sp<Vec2>::make<Vec2Impl>(_x->build(args), _y.build(args));
}

}
