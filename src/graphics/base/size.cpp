#include "graphics/base/size.h"

#include "core/ark.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/bean_utils.h"

#include "graphics/base/v2.h"

namespace ark {

Size::Size()
    : _width(sp<NumericWrapper>::make(0.0f)), _height(sp<NumericWrapper>::make(0.0f))
{
}

Size::Size(float width, float height)
    : _width(sp<NumericWrapper>::make(width)), _height(sp<NumericWrapper>::make(height))
{
}

Size::Size(const sp<Numeric>& vwidth, const sp<Numeric>& vheight)
    : _width(sp<NumericWrapper>::make(vwidth)), _height(sp<NumericWrapper>::make(vheight))
{
}

V Size::val()
{
    return V(_width->val(), _height->val());
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

const sp<Numeric> Size::vwidth() const
{
    return _width;
}

const sp<Numeric> Size::vheight() const
{
    return _height;
}

void Size::assign(const Size& other)
{
    _width = other._width;
    _height = other._height;
}

void Size::adopt(const Size& other)
{
    _width->set(other._width);
    _height->set(other._height);
}

template<> ARK_API const sp<Size> Null::ptr()
{
    return Ark::instance().obtain<Size>();
}

Size::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
{
    BeanUtils::parse(parent, value, _width, _height);
}

sp<Size> Size::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Size>::make(_width->build(args), _height->build(args));
}

Size::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _size(parent.getBuilder<Size>(manifest, Constants::Attributes::SIZE, false)),
      _width(_size ? sp<Builder<Numeric>>::null() : parent.getBuilder<Numeric>(manifest, Constants::Attributes::WIDTH)),
      _height(_size ? sp<Builder<Numeric>>::null() : parent.getBuilder<Numeric>(manifest, Constants::Attributes::HEIGHT))
{
}

sp<Size> Size::BUILDER::build(const sp<Scope>& args)
{
    if(_size)
        return _size->build(args);
    return sp<Size>::make(_width->build(args), _height->build(args));
}

}
