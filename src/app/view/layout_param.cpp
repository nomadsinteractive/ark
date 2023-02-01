#include "app/view/layout_param.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/string_convert.h"
#include "core/util/strings.h"
#include "core/util/updatable_util.h"

#include "graphics/base/size.h"

namespace ark {

template<> ARK_API LayoutParam::Display StringConvert::to<String, LayoutParam::Display>(const String& str)
{
    if(str == "float")
        return LayoutParam::DISPLAY_FLOAT;
    if(str == "absolute")
        return LayoutParam::DISPLAY_ABSOLUTE;
    return LayoutParam::DISPLAY_BLOCK;
}

LayoutParam::LayoutParam(const sp<Size>& size, Display display, Gravity gravity, float grow)
    : _width_type(LayoutParam::LENGTH_TYPE_PIXEL), _height_type(LayoutParam::LENGTH_TYPE_PIXEL), _size(Null::toSafePtr(size)), _display(display), _gravity(gravity), _flex_grow(grow)
{
}

LayoutParam::LayoutParam(Length width, Length height, FlexDirection flexDirection, FlexWrap flexWrap, JustifyContent justifyContent, Align alignItems, Align alignSelf,
                         Align alignContent, Display display, float flexGrow, Length flexBasis, sp<Vec4> margins, sp<Vec4> paddings, sp<Vec3> position)
    : _width_type(width._type), _height_type(height._type), _size(sp<Size>::make(width._value, height._value)), _size_min(nullptr, V3(NAN)), _size_max(nullptr, V3(NAN)),
      _flex_direction(flexDirection), _flex_wrap(flexWrap), _justify_content(justifyContent), _align_items(alignItems), _align_self(alignSelf), _align_content(alignContent),
      _display(display), _flex_basis(std::move(flexBasis)), _flex_grow(flexGrow), _margins(std::move(margins)), _paddings(std::move(paddings)), _position(std::move(position))
{
}

bool LayoutParam::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp) || UpdatableUtil::update(timestamp, _size, _size_min, _size_max, _margins, _paddings, _flex_basis);
}

float LayoutParam::calcLayoutWidth(float available) const
{
    const V4 margins = _margins.val();
    if(isMatchParent(_size->widthAsFloat()))
    {
        _size->setWidth(available - margins.w() - margins.y());
        return available;
    }
    return _size->widthAsFloat() + margins.w() + margins.y();
}

float LayoutParam::calcLayoutHeight(float available) const
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

const sp<Size>& LayoutParam::size() const
{
    return _size;
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

LayoutParam::LengthType LayoutParam::flexBasisType() const
{
    return _flex_basis._type;
}

void LayoutParam::setFlexBasisType(LengthType basisType)
{
    _flex_basis._type = basisType;
    _timestamp.markDirty();
}

sp<Numeric> LayoutParam::flexBasis() const
{
    return _flex_basis._value.ensure();
}

void LayoutParam::setFlexBasis(sp<Numeric> flexBasis)
{
    _flex_basis._value = std::move(flexBasis);
}

float LayoutParam::flexGrow() const
{
    return _flex_grow;
}

void LayoutParam::setFlexGrow(float weight)
{
    _flex_grow = weight;
}

bool LayoutParam::hasFlexGrow() const
{
    return _flex_grow != 0.0f;
}

sp<Numeric> LayoutParam::width() const
{
    return _size->width();
}

void LayoutParam::setWidth(sp<Numeric> width)
{
    _size->setWidth(std::move(width));
}

LayoutParam::LengthType LayoutParam::widthType() const
{
    return _width_type;
}

void LayoutParam::setWidthType(LengthType widthType)
{
    _width_type = widthType;
    _timestamp.markDirty();
}

sp<Numeric> LayoutParam::height() const
{
    return _size->height();
}

void LayoutParam::setHeight(sp<Numeric> height)
{
    _size->setHeight(std::move(height));
}

LayoutParam::LengthType LayoutParam::heightType() const
{
    return _height_type;
}

void LayoutParam::setHeightType(LengthType heightType)
{
    _height_type = heightType;
    _timestamp.markDirty();
}

LayoutParam::FlexDirection LayoutParam::flexDirection() const
{
    return _flex_direction;
}

LayoutParam::FlexWrap LayoutParam::flexWrap() const
{
    return _flex_wrap;
}

LayoutParam::JustifyContent LayoutParam::justifyContent() const
{
    return _justify_content;
}

LayoutParam::Align LayoutParam::alignItems() const
{
    return _align_items;
}

LayoutParam::Align LayoutParam::alignSelf() const
{
    return _align_self;
}

LayoutParam::Align LayoutParam::alignContent() const
{
    return _align_content;
}

const SafeVar<Vec4>& LayoutParam::margins() const
{
    return _margins;
}

void LayoutParam::setMargins(sp<Vec4> margins)
{
    _margins = std::move(margins);
}

const SafeVar<Vec4>& LayoutParam::paddings() const
{
    return _paddings;
}

void LayoutParam::setPaddings(sp<Vec4> paddings)
{
    _paddings.reset(std::move(paddings));
}

const SafeVar<Vec3>& LayoutParam::position() const
{
    return _position;
}

void LayoutParam::setPosition(sp<Vec3> position)
{
    _position.reset(std::move(position));
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

namespace {

class BuilderLengthVar : public Builder<LayoutParam::Length> {
public:
    BuilderLengthVar(LayoutParam::LengthType varType, sp<Builder<Numeric>> var)
        : _var_type(varType), _var(std::move(var)) {
    }

    virtual sp<LayoutParam::Length> build(const Scope& args) override {
        return sp<LayoutParam::Length>::make(_var_type, _var->build(args));
    }

private:
    LayoutParam::LengthType _var_type;
    sp<Builder<Numeric>> _var;
};

}

static sp<Builder<LayoutParam::Length>> getLengthBuilder(BeanFactory& factory, const document& manifest, const String& attrName) {
    const Optional<String> attrOpt = Documents::getAttributeOptional<String>(manifest, attrName);
    if(attrOpt) {
        const String& s = attrOpt.value();
        if(s == "auto")
            return sp<typename Builder<LayoutParam::Length>::Prebuilt>::make(sp<LayoutParam::Length>::make());
        if(s.endsWith("px"))
            return sp<typename Builder<LayoutParam::Length>::Prebuilt>::make(sp<LayoutParam::Length>::make(LayoutParam::LENGTH_TYPE_PIXEL, Strings::parse<float>(s.substr(0, s.length() - 2))));
        if(s.endsWith("%"))
            return sp<typename Builder<LayoutParam::Length>::Prebuilt>::make(sp<LayoutParam::Length>::make(LayoutParam::LENGTH_TYPE_PERCENTAGE, Strings::parse<float>(s.substr(0, s.length() - 1))));
        return sp<BuilderLengthVar>::make(LayoutParam::LENGTH_TYPE_PIXEL, factory.ensureBuilder<Numeric>(s));
    }
    return nullptr;
}

LayoutParam::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _width(getLengthBuilder(factory, manifest, "width")), _height(getLengthBuilder(factory, manifest, "height")),
      _flex_direction(Documents::getAttribute<FlexDirection>(manifest, "flex-direction", FLEX_DIRECTION_ROW)), _flex_wrap(Documents::getAttribute<FlexWrap>(manifest, "flex-wrap", FLEX_WRAP_NOWRAP)),
      _justify_content(Documents::getAttribute<JustifyContent>(manifest, "justify-content", JUSTIFY_CONTENT_FLEX_START)), _align_items(Documents::getAttribute<Align>(manifest, "align-items", ALIGN_STRETCH)),
      _align_self(Documents::getAttribute<Align>(manifest, "align-self", ALIGN_AUTO)), _align_content(Documents::getAttribute<Align>(manifest, "align-content", ALIGN_FLEX_START)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)), _display(Documents::getAttribute<Display>(manifest, "display", LayoutParam::DISPLAY_BLOCK)),
      _flex_grow(Documents::getAttribute<float>(manifest, "flex-grow", 0.0)), _margins(factory.getBuilder<Vec4>(manifest, "margins")), _paddings(factory.getBuilder<Vec4>(manifest, "paddings")),
      _position(factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION))
{
}

sp<LayoutParam> LayoutParam::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size ? _size->build(args) : nullptr;
    sp<Vec4> margins = _margins->build(args);
    sp<Vec4> paddings = _paddings->build(args);
    sp<Vec3> position = _position->build(args);
    if(size)
        return sp<LayoutParam>::make(Length(LENGTH_TYPE_PIXEL, size->width()), Length(LENGTH_TYPE_PIXEL, size->height()), _flex_direction, _flex_wrap, _justify_content, _align_items, _align_self,
                                     _align_content, _display, _flex_grow, LayoutParam::Length(), std::move(margins), std::move(paddings), std::move(position));
    if(_width || _height)
        return sp<LayoutParam>::make(_width ? _width->build(args) : Length(), _height ? _height->build(args) : Length(), _flex_direction, _flex_wrap, _justify_content, _align_items, _align_self,
                                     _align_content, _display, _flex_grow, LayoutParam::Length(), std::move(margins), std::move(paddings), std::move(position));
    return nullptr;
}

template<> ARK_API sp<LayoutParam> Null::safePtr()
{
    return sp<LayoutParam>::make(sp<Size>::make(static_cast<float>(LayoutParam::SIZE_CONSTRAINT_MATCH_PARENT), static_cast<float>(LayoutParam::SIZE_CONSTRAINT_MATCH_PARENT)));
}

template<> ARK_API LayoutParam::Gravity StringConvert::to<String, LayoutParam::Gravity>(const String& s)
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

template<> ARK_API LayoutParam::FlexDirection StringConvert::to<String, LayoutParam::FlexDirection>(const String& s)
{
    if(s == "column")
        return LayoutParam::FLEX_DIRECTION_COLUMN;
    if(s == "column_reverse")
        return LayoutParam::FLEX_DIRECTION_COLUMN_REVERSE;

    if(s == "row")
        return LayoutParam::FLEX_DIRECTION_ROW;

    CHECK(s == "row_reverse", "Unknow enum %s(\"%s\"), possible values are: [%s]", s.c_str(), "FlexDirection", "'column', 'column_reverse', 'row', 'row_reverse'");
    return LayoutParam::FLEX_DIRECTION_ROW_REVERSE;
}

template<> ARK_API LayoutParam::FlexWrap StringConvert::to<String, LayoutParam::FlexWrap>(const String& s)
{
    if(s == "nowrap")
        return LayoutParam::FLEX_WRAP_NOWRAP;
    if(s == "wrap")
        return LayoutParam::FLEX_WRAP_WRAP;
    CHECK(s == "wrap_reverse", "Unknow enum %s(\"%s\"), possible values are: [%s]", s.c_str(), "FlexWrap", "'nowrap', 'wrap_reverse'");
    return LayoutParam::FLEX_WRAP_WRAP_REVERSE;
}

template<> ARK_API LayoutParam::JustifyContent StringConvert::to<String, LayoutParam::JustifyContent>(const String& s)
{
    if(s == "flex_start")
        return LayoutParam::JUSTIFY_CONTENT_FLEX_START;
    if(s == "flex_end")
        return LayoutParam::JUSTIFY_CONTENT_FLEX_END;
    if(s == "center")
        return LayoutParam::JUSTIFY_CONTENT_CENTER;
    if(s == "space_between")
        return LayoutParam::JUSTIFY_CONTENT_SPACE_BETWEEN;
    if(s == "space_around")
        return LayoutParam::JUSTIFY_CONTENT_SPACE_AROUND;
    CHECK(s == "space_evently", "Unknow enum %s(\"%s\"), possible values are: [%s]", s.c_str(), "JustifyContent", "'flex_start', 'flex_end', 'center', 'space_between', 'space_around', 'space_evently'");
    return LayoutParam::JUSTIFY_CONTENT_SPACE_EVENLY;
}

template<> ARK_API LayoutParam::Align StringConvert::to<String, LayoutParam::Align>(const String& s)
{
    if(s == "auto")
        return LayoutParam::ALIGN_AUTO;
    if(s == "flex_start")
        return LayoutParam::ALIGN_FLEX_START;
    if(s == "flex_end")
        return LayoutParam::ALIGN_FLEX_END;
    if(s == "center")
        return LayoutParam::ALIGN_CENTER;
    if(s == "stretch")
        return LayoutParam::ALIGN_STRETCH;
    if(s == "baseline")
        return LayoutParam::ALIGN_BASELINE;
    if(s == "space_between")
        return LayoutParam::ALIGN_SPACE_BETWEEN;
    CHECK(s == "space_around", "Unknow enum %s(\"%s\"), possible values are: [%s]", s.c_str(), "Align", "'auto', 'flex_start', 'flex_end', 'center', 'stretch', 'baseline', 'space_between', 'space_around'");
    return LayoutParam::ALIGN_SPACE_AROUND;
}

LayoutParam::Length::Length()
    : _type(LENGTH_TYPE_AUTO), _value(nullptr, 0)
{
}

LayoutParam::Length::Length(LengthType type, float value)
    : _type(type), _value(sp<Numeric::Const>::make(value))
{
}

LayoutParam::Length::Length(LengthType type, sp<Numeric> value)
    : _type(type), _value(std::move(value))
{
}

bool LayoutParam::Length::update(uint64_t timestamp) const
{
    return _value.update(timestamp);
}

}
