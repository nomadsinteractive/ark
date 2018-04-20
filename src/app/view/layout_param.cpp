#include "app/view/layout_param.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/strings.h"

#include "graphics/base/size.h"

namespace ark {

template<> ARK_API LayoutParam::Display Conversions::to<String, LayoutParam::Display>(const String& str)
{
    if(str == "float")
        return LayoutParam::DISPLAY_FLOAT;
    if(str == "absolute")
        return LayoutParam::DISPLAY_ABSOLUTE;
    return LayoutParam::DISPLAY_BLOCK;
}

LayoutParam::LayoutParam(const sp<Size>& size, LayoutParam::Display display)
    : _size(Null::toSafe(size)), _display(display)
{
}

float LayoutParam::calcLayoutWidth(float available)
{
    if(isMatchParent(_size->width()))
    {
        _size->setWidth(available - _margins.left() - _margins.right());
        return available;
    }
    return _size->width() + _margins.left() + _margins.right();
}

float LayoutParam::calcLayoutHeight(float available)
{
    if(isMatchParent(_size->height()))
    {
        _size->setHeight(available - _margins.top() - _margins.bottom());
        return available;
    }
    return _size->height() + _margins.top() + _margins.bottom();
}

float LayoutParam::contentWidth() const
{
    return _size->width();
}

void LayoutParam::setContentWidth(float contentWidth)
{
    _size->setWidth(contentWidth);
}

float LayoutParam::contentHeight() const
{
    return _size->height();
}

void LayoutParam::setContentHeight(float contentHeight)
{
    _size->setHeight(contentHeight);
}

const sp<Size>& LayoutParam::size() const
{
    return _size;
}

const void LayoutParam::setSize(const sp<Size>& size)
{
    _size = size;
}

LayoutParam::Display LayoutParam::display() const
{
    return _display;
}

void LayoutParam::setDisplay(Display display)
{
    _display = display;
}

const Rect& LayoutParam::margins() const
{
    return _margins;
}

Rect& LayoutParam::margins()
{
    return _margins;
}

bool LayoutParam::isWrapContent() const
{
    return _size->width() == -2.0f || _size->height() == -2.0f;
}

bool LayoutParam::isMatchParent(float unit)
{
    return unit == -1.0f;
}

bool LayoutParam::isWrapContent(float unit)
{
    return unit == -2.0f;
}

sp<Size> LayoutParam::parseSize(BeanFactory& beanFactory, const String& value, const sp<Scope>& args)
{
    const Identifier id = Identifier::parse(value);
    if(id.isArg() || id.isRef())
        return beanFactory.ensure<Size>(value, args);

    String width, height;
    Strings::cut(Strings::unwrap(value, '(', ')'), width, height, ',');
    return sp<Size>::make(getUnit(beanFactory, width.strip(), args), getUnit(beanFactory, height.strip(), args));
}

const sp<Numeric> LayoutParam::getUnit(BeanFactory& beanFactory, const String& value, const sp<Scope>& args)
{
    if(value == "match_parent")
        return sp<Numeric::Impl>::make(-1.0f);
    if(value == "wrap_content")
        return sp<Numeric::Impl>::make(-2.0f);
    return beanFactory.ensure<Numeric>(value, args);
}

LayoutParam::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _bean_factory(parent), _size(Documents::getAttribute(doc, Constants::Attributes::SIZE)), _display(Documents::getAttribute<Display>(doc, "display", LayoutParam::DISPLAY_BLOCK))
{
}

sp<LayoutParam> LayoutParam::BUILDER::build(const sp<Scope>& args)
{
    return _size ? sp<LayoutParam>::make(LayoutParam::parseSize(_bean_factory, _size, args), _display) : nullptr;
}

}
