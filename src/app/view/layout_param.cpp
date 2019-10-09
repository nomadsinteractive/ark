#include "app/view/layout_param.h"

#include "core/ark.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/conversions.h"
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

LayoutParam::LayoutParam(const sp<Size>& size, LayoutParam::Display display, Gravity gravity)
    : _size(Null::toSafe(size)), _display(display), _gravity(gravity)
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
    return std::max(_size->width(), 0.0f);
}

float LayoutParam::offsetWidth() const
{
    return contentWidth() + _margins.left() + _margins.right();
}

void LayoutParam::setContentWidth(float contentWidth)
{
    _size->setWidth(contentWidth);
}

float LayoutParam::contentHeight() const
{
    return std::max(_size->height(), 0.0f);
}

float LayoutParam::offsetHeight() const
{
    return contentHeight() + _margins.top() + _margins.bottom();
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

LayoutParam::Gravity LayoutParam::gravity() const
{
    return _gravity;
}

void LayoutParam::setGravity(LayoutParam::Gravity gravity)
{
    _gravity = gravity;
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

sp<LayoutParam> LayoutParam::BUILDER::build(const Scope& args)
{
    return _size ? sp<LayoutParam>::make(_size->build(args), _display) : nullptr;
}

template<> ARK_API sp<LayoutParam> Null::ptr()
{
    return Ark::instance().obtain<LayoutParam>(Ark::instance().obtain<Size>(static_cast<float>(LayoutParam::MATCH_PARENT), static_cast<float>(LayoutParam::MATCH_PARENT)));
}

template<> ARK_API LayoutParam::Gravity Conversions::to<String, LayoutParam::Gravity>(const String& s)
{
    if(s == "none")
        return LayoutParam::NONE;

    uint32_t gravity = LayoutParam::NONE;
    for(const String& i : s.split('|'))
    {
        const String str = i.strip();
        if(str == "left")
            gravity |= LayoutParam::LEFT;
        else if(str == "right")
            gravity |= LayoutParam::RIGHT;
        else if(str == "top")
            gravity |= LayoutParam::TOP;
        else if(str == "bottom")
            gravity |= LayoutParam::BOTTOM;
        else if(str == "center")
            gravity |= LayoutParam::CENTER;
        else if(str == "center_horizontal")
            gravity |= LayoutParam::CENTER_HORIZONTAL;
        else if(str == "center_vertical")
            gravity |= LayoutParam::CENTER_VERTICAL;
        else
            DFATAL("Unknown gravity value: \"%s\"", i.c_str());
    }
    return static_cast<LayoutParam::Gravity>(gravity);
}

}
