#include "graphics/components/size.h"

#include "core/ark.h"
#include "core/base/constants.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/global.h"

#include "graphics/base/v2.h"

namespace ark {

Size::Size()
    : _impl(sp<Vec3Impl>::make())
{
}

Size::Size(const V3 size)
    : Size(size.x(), size.y(), size.z())
{
}

Size::Size(float width, float height, float depth)
    : _impl(sp<Vec3Impl>::make(width, height, depth))
{
}

Size::Size(sp<Numeric> width, sp<Numeric> height, sp<Numeric> depth)
    : _impl(sp<Vec3Impl>::make(std::move(width), std::move(height), depth ? std::move(depth) : Global<Constants>()->NUMERIC_ZERO))
{
}

V3 Size::val()
{
    return _impl->val();
}

bool Size::update(uint32_t tick)
{
    return _impl->update(tick);
}

float Size::widthAsFloat() const
{
    return _impl->x()->val();
}

void Size::setWidth(const float width)
{
    _impl->x()->set(width);
}

void Size::setWidth(sp<Numeric> width)
{
    _impl->x()->set(std::move(width));
}

float Size::heightAsFloat() const
{
    return _impl->y()->val();
}

void Size::setHeight(const float height)
{
    _impl->y()->set(height);
}

void Size::setHeight(sp<Numeric> height)
{
    _impl->y()->set(std::move(height));
}

float Size::depthAsFloat() const
{
    return _impl->z()->val();
}

void Size::setDepth(const float depth)
{
    _impl->z()->set(depth);
}

void Size::setDepth(sp<Numeric> depth)
{
    _impl->z()->set(std::move(depth));
}

sp<Numeric> Size::width() const
{
    return _impl->x();
}

sp<Numeric> Size::height() const
{
    return _impl->y();
}

sp<Numeric> Size::depth() const
{
    return _impl->z();
}

void Size::reset(const Size& other)
{
    setWidth(other.width());
    setHeight(other.height());
    setDepth(other.depth());
}

sp<Size> Size::freeze()
{
    _impl->update(Timestamp::now());
    return sp<Size>::make(val());
}

const sp<Vec3Impl>& Size::impl() const
{
    return _impl;
}

Size::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _expr(expr)
{
    factory.expand(expr, _width, _height, _depth);
}

sp<Size> Size::DICTIONARY::build(const Scope& args)
{
    sp<Numeric> w = _width->build(args);
    sp<Numeric> h = _height->build(args);
    sp<Numeric> d = _depth->build(args);
    CHECK(w && h, "Cannot build Size from \"%s\"", _expr.c_str());
    return sp<Size>::make(std::move(w), std::move(h), std::move(d));
}

Size::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _size(factory.getBuilder<Size>(manifest, constants::SIZE)),
      _width(_size ? sp<Builder<Numeric>>() : factory.getBuilder<Numeric>(manifest, constants::WIDTH)),
      _height(_size ? sp<Builder<Numeric>>() : factory.getBuilder<Numeric>(manifest, constants::HEIGHT)),
      _depth(_size ? sp<Builder<Numeric>>() : factory.getBuilder<Numeric>(manifest, constants::DEPTH))
{
}

sp<Size> Size::BUILDER::build(const Scope& args)
{
    if(_size)
        return _size->build(args);
    return sp<Size>::make(_width.build(args), _height.build(args), _depth.build(args));
}

}
