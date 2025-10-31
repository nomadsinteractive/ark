#include "graphics/impl/vec/vec3_impl.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/v3.h"
#include "graphics/util/vec3_type.h"

namespace ark {

Vec3Impl::Vec3Impl() noexcept
    : Vec3Impl(0, 0, 0)
{
}

Vec3Impl::Vec3Impl(sp<Numeric> v) noexcept
    : _x(sp<NumericWrapper>::make(v)), _y(sp<NumericWrapper>::make(v)), _z(sp<NumericWrapper>::make(std::move(v)))
{
}

Vec3Impl::Vec3Impl(float x, float y, float z) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z))
{
}

Vec3Impl::Vec3Impl(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z) noexcept
    : _x(sp<NumericWrapper>::make(std::move(x))), _y(sp<NumericWrapper>::make(std::move(y))), _z(sp<NumericWrapper>::make(std::move(z)))
{
}

V3 Vec3Impl::val()
{
    return {_x->val(), _y->val(), _z->val()};
}

bool Vec3Impl::update(uint32_t tick)
{
    return UpdatableUtil::update(tick, _x, _y, _z);
}

const sp<NumericWrapper>& Vec3Impl::x() const
{
    return _x;
}

const sp<NumericWrapper>& Vec3Impl::y() const
{
    return _y;
}

const sp<NumericWrapper>& Vec3Impl::z() const
{
    return _z;
}

void Vec3Impl::set(const V3 val) const
{
    _x->set(val.x());
    _y->set(val.y());
    _z->set(val.z());
}

Vec3Impl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _x(factory.ensureBuilder<Numeric>(manifest, "x")), _y(factory.getBuilder<Numeric>(manifest, "y")), _z(factory.getBuilder<Numeric>(manifest, "z"))
{
}

sp<Vec3> Vec3Impl::BUILDER::build(const Scope& args)
{
    return Vec3Type::create(_x.build(args), _y.build(args), _z.build(args));
}

Vec3Impl::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _expr(expr)
{
    factory.expand(expr, _x, _y, _z);
}

sp<Vec3> Vec3Impl::DICTIONARY::build(const Scope& args)
{
    sp<Numeric> x = _x->build(args);
    sp<Numeric> y = _y->build(args);
    sp<Numeric> z = _z->build(args);
    CHECK(x, "Cannot build Vec3 from \"%s\"", _expr.c_str());
    return Vec3Type::create(std::move(x), std::move(y), std::move(z));
}

}
