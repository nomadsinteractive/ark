#include "app/view/layout_param.h"

#include "core/ark.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/conversions.h"
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

LayoutParam::LayoutParam(const sp<Size>& size, LayoutParam::Display display, Gravity gravity, float weight)
    : _size(Null::toSafe(size)), _margins(nullptr), _display(display), _gravity(gravity), _weight(weight)
{
}

float LayoutParam::calcLayoutWidth(float available)
{
    const V4 margins = _margins.val();
    if(isMatchParent(_size->widthAsFloat()))
    {
        _size->setWidth(available - margins.w() - margins.y());
        return available;
    }
    return _size->widthAsFloat() + margins.w() + margins.y();
}

float LayoutParam::calcLayoutHeight(float available)
{
    const V4 margins = _margins.val();
    if(isMatchParent(_size->heightAsFloat()))
    {
        _size->setHeight(available - margins.x() - margins.z());
        return available;
    }
    return _size->heightAsFloat() + margins.x() + margins.z();
}

float LayoutParam::contentWidth() const
{
    return std::max(_size->widthAsFloat(), 0.0f);
}

float LayoutParam::offsetWidth() const
{
    const V4 margins = _margins.val();
    return contentWidth() + margins.w() + margins.y();
}

void LayoutParam::setContentWidth(float contentWidth)
{
    _size->setWidth(contentWidth);
}

float LayoutParam::contentHeight() const
{
    return std::max(_size->heightAsFloat(), 0.0f);
}

float LayoutParam::offsetHeight() const
{
    const V4 margins = _margins.val();
    return contentHeight() + margins.x() + margins.z();
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

const sp<Boolean>& LayoutParam::stopPropagation() const
{
    return _stop_propagation;
}

void LayoutParam::setStopPropagation(sp<Boolean> stopPropagation)
{
    _stop_propagation = std::move(stopPropagation);
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

float LayoutParam::weight() const
{
    return _weight;
}

void LayoutParam::setWeight(float weight)
{
    _weight = weight;
}

bool LayoutParam::hasWeight() const
{
    return _weight != 0.0f;
}

const SafeVar<Vec4>& LayoutParam::margins() const
{
    return _margins;
}

void LayoutParam::setMargins(sp<Vec4> margins)
{
    _margins = std::move(margins);
}

bool LayoutParam::isWrapContent() const
{
    return isWidthWrapContent() || isHeightWrapContent();
}

bool LayoutParam::isWidthWrapContent() const
{
    return isWrapContent(_size->widthAsFloat());
}

bool LayoutParam::isHeightWrapContent() const
{
    return isWrapContent(_size->heightAsFloat());
}

bool LayoutParam::isMatchParent() const
{
    return isWidthMatchParent() || isHeightMatchParent();
}

bool LayoutParam::isWidthMatchParent() const
{
    return isMatchParent(_size->widthAsFloat());
}

bool LayoutParam::isHeightMatchParent() const
{
    return isMatchParent(_size->heightAsFloat());
}

bool LayoutParam::isMatchParent(float unit)
{
    return static_cast<int32_t>(unit) == SIZE_CONSTRAINT_MATCH_PARENT;
}

bool LayoutParam::isWrapContent(float unit)
{
    return static_cast<int32_t>(unit) == SIZE_CONSTRAINT_WRAP_CONTENT;
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
    return sp<LayoutParam>::make(sp<Size>::make(static_cast<float>(LayoutParam::SIZE_CONSTRAINT_MATCH_PARENT), static_cast<float>(LayoutParam::SIZE_CONSTRAINT_MATCH_PARENT)));
}

template<> ARK_API LayoutParam::Gravity Conversions::to<String, LayoutParam::Gravity>(const String& s)
{
    if(s == "default")
        return LayoutParam::GRAVITY_DEFAULT;

    uint32_t gravity = 0;
    for(const String& i : s.split('|'))
    {
        const String str = i.strip();
        if(str == "left")
            gravity |= LayoutParam::GRAVITY_LEFT;
        else if(str == "right")
            gravity |= LayoutParam::GRAVITY_RIGHT;
        else if(str == "top")
            gravity |= LayoutParam::GRAVITY_TOP;
        else if(str == "bottom")
            gravity |= LayoutParam::GRAVITY_BOTTOM;
        else if(str == "center")
            gravity |= LayoutParam::GRAVITY_CENTER;
        else if(str == "center_horizontal")
            gravity |= LayoutParam::GRAVITY_CENTER_HORIZONTAL;
        else if(str == "center_vertical")
            gravity |= LayoutParam::GRAVITY_CENTER_VERTICAL;
        else
            DFATAL("Unknown gravity value: \"%s\"", i.c_str());
    }
    return static_cast<LayoutParam::Gravity>(gravity);
}

template<> ARK_API LayoutParam::Length Conversions::to<String, LayoutParam::Length>(const String& s)
{
    if(s == "auto")
        return LayoutParam::Length();

    if(s.endsWith("px"))
        return LayoutParam::Length(LayoutParam::LENGTH_TYPE_PIXEL, Strings::parse<float>(s.substr(0, s.length() - 2)));
    if(s.endsWith("%"))
        return LayoutParam::Length(LayoutParam::LENGTH_TYPE_PERCENTAGE, Strings::parse<float>(s.substr(0, s.length() - 1)));

    return LayoutParam::Length(LayoutParam::LENGTH_TYPE_PIXEL, Strings::parse<float>(s));
}

LayoutParam::Length::Length()
    : _type(LENGTH_TYPE_AUTO), _value(0)
{
}

LayoutParam::Length::Length(LengthType type, float value)
    : _type(type), _value(value)
{
}

}
