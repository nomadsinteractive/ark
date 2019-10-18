#include "graphics/impl/vec/vec4_impl.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"

#include "graphics/base/color.h"

namespace ark {

Vec4Impl::Vec4Impl() noexcept
    : Vec4Impl(0, 0, 0, 0)
{
}

Vec4Impl::Vec4Impl(float x, float y, float z, float w) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z)), _w(sp<NumericWrapper>::make(w))
{
}

Vec4Impl::Vec4Impl(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z)), _w(sp<NumericWrapper>::make(w))
{
}

V4 Vec4Impl::val()
{
    return V4(_x->val(), _y->val(), _z->val(), _w->val());
}

void Vec4Impl::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_x, visitor);
    HolderUtil::visit(_y, visitor);
    HolderUtil::visit(_z, visitor);
    HolderUtil::visit(_w, visitor);
}

const sp<NumericWrapper>& Vec4Impl::x() const
{
    return _x;
}

const sp<NumericWrapper>& Vec4Impl::y() const
{
    return _y;
}

const sp<NumericWrapper>& Vec4Impl::z() const
{
    return _z;
}

const sp<NumericWrapper>& Vec4Impl::w() const
{
    return _w;
}

void Vec4Impl::set(const V4& val)
{
    _x->set(val.x());
    _y->set(val.y());
    _z->set(val.z());
    _w->set(val.w());
}

void Vec4Impl::fix()
{
    _x->fix();
    _y->fix();
    _z->fix();
    _w->fix();
}

Vec4Impl::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _x(factory.ensureBuilder<Numeric>(manifest, "x")), _y(factory.getBuilder<Numeric>(manifest, "y")), _z(factory.getBuilder<Numeric>(manifest, "z")), _w(factory.getBuilder<Numeric>(manifest, "w"))
{
}

sp<Vec4> Vec4Impl::BUILDER::build(const Scope& args)
{
    return sp<Vec4Impl>::make(_x->build(args), _y->build(args), _z->build(args), _w->build(args));
}

Vec4Impl::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
    : _is_color(str.startsWith("#"))
{
    if(_is_color)
        _color = Strings::parse<Color>(str);
    else
        BeanUtils::split(factory, str, _x, _y, _z, _w);
}

sp<Vec4> Vec4Impl::DICTIONARY::build(const Scope& args)
{
    if(_is_color)
        return sp<Color>::make(_color);
    return sp<Vec4Impl>::make(_x->build(args), _y->build(args), _z->build(args), _w->build(args));
}

}
