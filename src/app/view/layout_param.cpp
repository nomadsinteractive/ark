#include "app/view/layout_param.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/strings.h"

#include "graphics/base/size.h"

namespace ark {

const int32_t LayoutParam::MATCH_PARENT = -1;
const int32_t LayoutParam::WRAP_CONTENT = -2;

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

const SafePtr<Size>& LayoutParam::size() const
{
    return _size;
}

void LayoutParam::setSize(const sp<Size>& size)
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
    return isWrapContent(_size->width()) || isWrapContent(_size->height());
}

bool LayoutParam::isMatchParent(float unit)
{
    return static_cast<int32_t>(unit) == MATCH_PARENT;
}

bool LayoutParam::isWrapContent(float unit)
{
    return static_cast<int32_t>(unit) == WRAP_CONTENT;
}

LayoutParam::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)), _display(Documents::getAttribute<Display>(manifest, "display", LayoutParam::DISPLAY_BLOCK))
{
}

sp<LayoutParam> LayoutParam::BUILDER::build(const sp<Scope>& args)
{
    return _size ? sp<LayoutParam>::make(_size->build(args), _display) : nullptr;
}

}
