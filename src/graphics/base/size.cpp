#include "graphics/base/size.h"

#include "core/ark.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

#include "graphics/base/v2.h"

namespace ark {

Size::Size()
    : _impl(sp<Vec3Impl>::make())
{
}

Size::Size(float width, float height, float depth)
    : _impl(sp<Vec3Impl>::make(width, height, depth))
{
}

Size::Size(const sp<Numeric>& width, const sp<Numeric>& height, const sp<Numeric>& depth)
    : _impl(sp<Vec3Impl>::make(width, height, depth))
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

void Size::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_impl, visitor);
}

float Size::width() const
{
    return _impl->x()->val();
}

void Size::setWidth(float width)
{
    _impl->x()->set(width);
}

void Size::setWidth(const sp<Numeric>& width)
{
    _impl->x()->set(width);
}

float Size::height() const
{
    return _impl->y()->val();
}

void Size::setHeight(float height)
{
    _impl->y()->set(height);
}

void Size::setHeight(const sp<Numeric>& height)
{
    _impl->y()->set(height);
}

float Size::depth() const
{
    return _impl->z()->val();
}

void Size::setDepth(float depth)
{
    _impl->z()->set(depth);
}

void Size::setDepth(const sp<Numeric>& depth)
{
    _impl->z()->set(depth);
}

const sp<Numeric> Size::vwidth() const
{
    return _impl->x();
}

const sp<Numeric> Size::vheight() const
{
    return _impl->y();
}

const sp<Numeric> Size::vdepth() const
{
    return _impl->z();
}

void Size::adopt(const Size& other)
{
    setWidth(other.vwidth());
    setHeight(other.vheight());
    setDepth(other.vdepth());
}

void Size::fix()
{
    _impl->fix();
}

const sp<Vec3Impl>& Size::impl() const
{
    return _impl;
}

template<> ARK_API sp<Size> Null::ptr()
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
      _width(_size ? sp<Builder<Numeric>>::null() : factory.getBuilder<Numeric>(manifest, Constants::Attributes::WIDTH)),
      _height(_size ? sp<Builder<Numeric>>::null() : factory.getBuilder<Numeric>(manifest, Constants::Attributes::HEIGHT)),
      _depth(_size ? sp<Builder<Numeric>>::null() : factory.getBuilder<Numeric>(manifest, Constants::Attributes::DEPTH))
{
}

sp<Size> Size::BUILDER::build(const Scope& args)
{
    if(_size)
        return _size->build(args);
    return sp<Size>::make(_width->build(args), _height->build(args), _depth->build(args));
}

}
