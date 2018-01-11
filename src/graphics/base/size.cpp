#include "graphics/base/size.h"

#include "core/impl/numeric/scalar.h"
#include "core/util/bean_utils.h"

#include "graphics/base/v2.h"

namespace ark {

Size::Size()
    : _width(sp<Scalar>::make(0.0f)), _height(sp<Scalar>::make(0.0f))
{
}

Size::Size(float width, float height)
    : _width(sp<Scalar>::make(width)), _height(sp<Scalar>::make(height))
{
}

Size::Size(const sp<Numeric>& vwidth, const sp<Numeric>& vheight)
    : _width(sp<Scalar>::make(vwidth)), _height(sp<Scalar>::make(vheight))
{
}

V2 Size::val()
{
    return V2(_width->val(), _height->val());
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

template<> ARK_API const sp<Size>& Null::ptr()
{
    static sp<Size> instance = sp<Size>::make();
    return instance;
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
