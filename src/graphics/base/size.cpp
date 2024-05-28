#include "graphics/base/size.h"

#include "core/ark.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
#include "core/util/updatable_util.h"

#include "graphics/base/v2.h"

namespace ark {

Size::Size()
    : _impl(sp<Vec3Impl>::make())
{
}

Size::Size(const V3& size)
    : Size(size.x(), size.y(), size.z())
{
}

Size::Size(float width, float height, float depth)
    : _impl(sp<Vec3Impl>::make(width, height, depth))
{
}

Size::Size(sp<Numeric> width, sp<Numeric> height, sp<Numeric> depth)
    : _impl(sp<Vec3Impl>::make(std::move(width), std::move(height), std::move(depth)))
{
}

V3 Size::val()
{
    return _impl->val();
}

bool Size::update(uint64_t timestamp)
{
    return _impl->update(timestamp);
}

float Size::widthAsFloat() const
{
    return _impl->x()->val();
}

void Size::setWidth(float width)
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

void Size::setHeight(float height)
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

void Size::setDepth(float depth)
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
    return sp<Size>::make(_impl->val());
}

const sp<Vec3Impl>& Size::impl() const
{
    return _impl;
}

template<> ARK_API sp<Size> Null::safePtr()
{
    return sp<Size>::make();
}

Size::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
{
    BeanUtils::split(factory, value, _width, _height, _depth);
}

sp<Size> Size::DICTIONARY::build(const Scope& args)
{
    return sp<Size>::make(_width->build(args), _height->build(args), _depth->build(args));
}

Size::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _width(_size ? sp<Builder<Numeric>>() : factory.getBuilder<Numeric>(manifest, Constants::Attributes::WIDTH)),
      _height(_size ? sp<Builder<Numeric>>() : factory.getBuilder<Numeric>(manifest, Constants::Attributes::HEIGHT)),
      _depth(_size ? sp<Builder<Numeric>>() : factory.getBuilder<Numeric>(manifest, Constants::Attributes::DEPTH))
{
}

sp<Size> Size::BUILDER::build(const Scope& args)
{
    if(_size)
        return _size->build(args);
    return sp<Size>::make(_width->build(args), _height->build(args), _depth->build(args));
}

}
