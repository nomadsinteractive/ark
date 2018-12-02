#include "graphics/base/size.h"

#include "core/ark.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"

#include "graphics/base/v2.h"

namespace ark {

Size::Size()
    : _width(sp<NumericWrapper>::make(0.0f)), _height(sp<NumericWrapper>::make(0.0f)), _depth(sp<NumericWrapper>::make(0.0f))
{
}

Size::Size(float width, float height, float depth)
    : _width(sp<NumericWrapper>::make(width)), _height(sp<NumericWrapper>::make(height)), _depth(sp<NumericWrapper>::make(depth))
{
}

Size::Size(const sp<Numeric>& width, const sp<Numeric>& height, const sp<Numeric>& depth)
    : _width(sp<NumericWrapper>::make(width)), _height(sp<NumericWrapper>::make(height)), _depth(sp<NumericWrapper>::make(depth))
{
}

V3 Size::val()
{
    return V3(_width->val(), _height->val(), _depth->val());
}

float Size::width() const
{
    return _width->val();
}

void Size::setWidth(float width)
{
    _width->set(width);
}

float Size::height() const
{
    return _height->val();
}

void Size::setHeight(float height)
{
    _height->set(height);
}

float Size::depth() const
{
    return _depth->val();
}

void Size::setDepth(float depth)
{
    _depth->set(depth);
}

const sp<Numeric> Size::vwidth() const
{
    return _width;
}

const sp<Numeric> Size::vheight() const
{
    return _height;
}

const sp<Numeric> Size::vdepth() const
{
    return _depth;
}

void Size::assign(const Size& other)
{
    _width = other._width;
    _height = other._height;
    _depth = other._depth;
}

void Size::adopt(const Size& other)
{
    _width->set(other._width);
    _height->set(other._height);
    _depth->set(other._depth);
}

template<> ARK_API sp<Size> Null::ptr()
{
    return Ark::instance().obtain<Size>();
}

Size::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
{
    BeanUtils::split(factory, value, _width, _height, _depth);
}

sp<Size> Size::DICTIONARY::build(const sp<Scope>& args)
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

sp<Size> Size::BUILDER::build(const sp<Scope>& args)
{
    if(_size)
        return _size->build(args);
    return sp<Size>::make(_width->build(args), _height->build(args), _depth->build(args));
}

}
