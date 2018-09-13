#include "graphics/impl/vec/vec3_impl.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"

#include "graphics/base/v3.h"

namespace ark {

Vec3Impl::Vec3Impl() noexcept
    : Vec3Impl(0, 0, 0)
{
}

Vec3Impl::Vec3Impl(float x, float y, float z) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z))
{
}

Vec3Impl::Vec3Impl(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z))
{
}

V3 Vec3Impl::val()
{
    return V3(_x->val(), _y->val(), _z->val());
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

void Vec3Impl::set(const V3& val)
{
    _x->set(val.x());
    _y->set(val.y());
    _z->set(val.z());
}

void Vec3Impl::fix()
{
    _x->fix();
    _y->fix();
    _z->fix();
}

Vec3Impl::BUILDER::BUILDER(BeanFactory& factory, const document& doc)
    : _x(factory.getBuilder<Numeric>(doc, "x")), _y(factory.getBuilder<Numeric>(doc, "y")), _z(factory.getBuilder<Numeric>(doc, "z"))
{
}

sp<Vec3> Vec3Impl::BUILDER::build(const sp<Scope>& args)
{
    return sp<Vec3Impl>::make(_x->build(args), _y->build(args), _z->build(args));
}

Vec3Impl::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
{
    BeanUtils::split(factory, str, _x, _y, _z);
}

sp<Vec3> Vec3Impl::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Vec3Impl>::make(_x->build(args), _y->build(args), _z->build(args));
}

}
