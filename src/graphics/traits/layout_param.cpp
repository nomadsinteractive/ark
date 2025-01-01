#include "graphics/traits/layout_param.h"

#include "core/base/constants.h"
#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/string_convert.h"
#include "core/util/strings.h"
#include "core/util/updatable_util.h"

#include "graphics/traits/size.h"

namespace ark {

namespace {

bool isLengthMatchParent(const LayoutParam::Length& length)
{
    return length._type == LayoutParam::LENGTH_TYPE_PERCENTAGE && length._value.val() == 100.0f;
}

bool isLengthWrapContent(const LayoutParam::Length& length)
{
    return length._type == LayoutParam::LENGTH_TYPE_AUTO;
}

class BuilderLengthVar final : public Builder<LayoutParam::Length> {
public:
    BuilderLengthVar(LayoutParam::LengthType varType, sp<Builder<Numeric>> var)
        : _var_type(varType), _var(std::move(var)) {
    }

    sp<LayoutParam::Length> build(const Scope& args) override {
        return sp<LayoutParam::Length>::make(_var_type, _var->build(args));
    }

private:
    LayoutParam::LengthType _var_type;
    sp<Builder<Numeric>> _var;
};

sp<Builder<LayoutParam::Length>> getLengthBuilder(BeanFactory& factory, const document& manifest, const String& attrName)
{
    if(const Optional<String> attrOpt = Documents::getAttributeOptional<String>(manifest, attrName))
    {
        const String& s = attrOpt.value();
        if(s == "auto")
            return sp<typename Builder<LayoutParam::Length>::Prebuilt>::make(sp<LayoutParam::Length>::make());
        if(s.endsWith("px"))
            return sp<typename Builder<LayoutParam::Length>::Prebuilt>::make(sp<LayoutParam::Length>::make(LayoutParam::LENGTH_TYPE_PIXEL, Strings::eval<float>(s.substr(0, s.length() - 2))));
        if(s.endsWith("%"))
            return sp<typename Builder<LayoutParam::Length>::Prebuilt>::make(sp<LayoutParam::Length>::make(LayoutParam::LENGTH_TYPE_PERCENTAGE, Strings::eval<float>(s.substr(0, s.length() - 1))));
        return sp<BuilderLengthVar>::make(LayoutParam::LENGTH_TYPE_PIXEL, factory.ensureBuilder<Numeric>(s));
    }
    return nullptr;
}

}

template<> ARK_API LayoutParam::Display StringConvert::eval<LayoutParam::Display>(const String& str)
{
    if(str == "float")
        return LayoutParam::DISPLAY_FLOAT;
    if(str == "absolute")
        return LayoutParam::DISPLAY_ABSOLUTE;
    return LayoutParam::DISPLAY_BLOCK;
}

LayoutParam::LayoutParam(Length width, Length height, sp<Layout> layout, FlexDirection flexDirection, FlexWrap flexWrap, JustifyContent justifyContent, Align alignItems, Align alignSelf,
                         Align alignContent, Display display, float flexGrow, Length flexBasis, sp<Vec4> margins, sp<Vec4> paddings, sp<Vec3> offset)
    : _width(std::move(width)), _height(std::move(height)), _layout(std::move(layout)), _flex_direction(flexDirection), _flex_wrap(flexWrap), _justify_content(justifyContent), _align_items(alignItems), _align_self(alignSelf), _align_content(alignContent),
      _display(display), _flex_basis(std::move(flexBasis)), _flex_grow(flexGrow), _margins(std::move(margins)), _paddings(std::move(paddings)), _offset(std::move(offset))
{
}

bool LayoutParam::update(uint64_t timestamp)
{
    return _timestamp.update(timestamp) || UpdatableUtil::update(timestamp, _width, _height, _margins, _paddings, _flex_basis);
}

const sp<Layout>& LayoutParam::layout() const
{
    return _layout;
}

void LayoutParam::setLayout(sp<Layout> layout)
{
    _layout = std::move(layout);
}

float LayoutParam::calcLayoutWidth(float available)
{
    const V4 margins = _margins.val();
    if(isLengthMatchParent(_width))
    {
        _width._value.reset(sp<Numeric>::make<Numeric::Const>(available - margins.w() - margins.y()));
        return available;
    }
    return _width._value.val() + margins.w() + margins.y();
}

float LayoutParam::calcLayoutHeight(float available)
{
    const V4 margins = _margins.val();
    if(isLengthMatchParent(_height))
    {
        _height._value.reset(sp<Numeric>::make<Numeric::Const>(available - margins.x() - margins.z()));
        return available;
    }
    return _height._value.val() + margins.x() + margins.z();
}

float LayoutParam::contentWidth() const
{
    return std::max(_width._value.val(), 0.0f);
}

float LayoutParam::occupyWidth() const
{
    const V4 margins = _margins.val();
    return contentWidth() + margins.w() + margins.y();
}

void LayoutParam::setContentWidth(float contentWidth)
{
    _width._value.reset(sp<Numeric::Const>::make(contentWidth));
}

float LayoutParam::contentHeight() const
{
    return std::max(_height._value.val(), 0.0f);
}

float LayoutParam::occupyHeight() const
{
    const V4 margins = _margins.val();
    return contentHeight() + margins.x() + margins.z();
}

void LayoutParam::setContentHeight(float contentHeight)
{
    _height._value.reset(sp<Numeric::Const>::make(contentHeight));
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

LayoutParam::LengthType LayoutParam::flexBasisType() const
{
    return _flex_basis._type;
}

void LayoutParam::setFlexBasisType(LengthType basisType)
{
    _flex_basis._type = basisType;
    _timestamp.markDirty();
}

const SafeVar<Numeric>& LayoutParam::flexBasis() const
{
    return _flex_basis._value;
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

const LayoutParam::Length& LayoutParam::width() const
{
    return _width;
}

void LayoutParam::setWidth(sp<Numeric> width)
{
    _width._value.reset(width);
}

LayoutParam::LengthType LayoutParam::widthType() const
{
    return _width._type;
}

void LayoutParam::setWidthType(LengthType widthType)
{
    _width._type = widthType;
    _timestamp.markDirty();
}

const LayoutParam::Length& LayoutParam::height() const
{
    return _height;
}

void LayoutParam::setHeight(sp<Numeric> height)
{
    _height._value.reset(std::move(height));
}

void LayoutParam::setHeightType(LengthType heightType)
{
    _height._type = heightType;
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

const SafeVar<Vec3>& LayoutParam::offset() const
{
    return _offset;
}

void LayoutParam::setOffset(sp<Vec3> offset)
{
    _offset.reset(std::move(offset));
}

bool LayoutParam::isWrapContent() const
{
    return isWidthWrapContent() || isHeightWrapContent();
}

bool LayoutParam::isWidthWrapContent() const
{
    return isLengthWrapContent(_width);
}

bool LayoutParam::isHeightWrapContent() const
{
    return isLengthWrapContent(_height);
}

bool LayoutParam::isMatchParent() const
{
    return isWidthMatchParent() || isHeightMatchParent();
}

bool LayoutParam::isWidthMatchParent() const
{
    return isLengthMatchParent(_width);
}

bool LayoutParam::isHeightMatchParent() const
{
    return isLengthMatchParent(_height);
}

LayoutParam::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _width(getLengthBuilder(factory, manifest, constants::WIDTH)), _height(getLengthBuilder(factory, manifest, constants::HEIGHT)), _layout(factory.getBuilder<Layout>(manifest, constants::LAYOUT)),
      _flex_direction(Documents::getAttribute<FlexDirection>(manifest, "flex-direction", FLEX_DIRECTION_ROW)), _flex_wrap(Documents::getAttribute<FlexWrap>(manifest, "flex-wrap", FLEX_WRAP_NOWRAP)),
      _justify_content(Documents::getAttribute<JustifyContent>(manifest, "justify-content", JUSTIFY_CONTENT_FLEX_START)), _align_items(Documents::getAttribute<Align>(manifest, "align-items", ALIGN_STRETCH)),
      _align_self(Documents::getAttribute<Align>(manifest, "align-self", ALIGN_AUTO)), _align_content(Documents::getAttribute<Align>(manifest, "align-content", ALIGN_FLEX_START)),
      _size(factory.getBuilder<Size>(manifest, constants::SIZE)), _display(Documents::getAttribute<Display>(manifest, "display", LayoutParam::DISPLAY_BLOCK)),
      _flex_grow(Documents::getAttribute<float>(manifest, "flex-grow", 0.0)), _margins(factory.getBuilder<Vec4>(manifest, "margins")), _paddings(factory.getBuilder<Vec4>(manifest, "paddings")),
      _offset(factory.getBuilder<Vec3>(manifest, "offset"))
{
}

sp<LayoutParam> LayoutParam::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size ? _size->build(args) : nullptr;
    sp<Vec4> margins = _margins.build(args);
    sp<Vec4> paddings = _paddings.build(args);
    sp<Vec3> offset = _offset.build(args);
    Length width = size ? Length(LENGTH_TYPE_PIXEL, size->width()) : _width ? _width->build(args) : Length();
    Length height = size ? Length(LENGTH_TYPE_PIXEL, size->height()) : _height ? _height->build(args) : Length();
    return sp<LayoutParam>::make(std::move(width), std::move(height), _layout.build(args), _flex_direction, _flex_wrap, _justify_content, _align_items, _align_self, _align_content, _display, _flex_grow,
                                 Length(), std::move(margins), std::move(paddings), std::move(offset));
}

template<> ARK_API LayoutParam::FlexDirection StringConvert::eval<LayoutParam::FlexDirection>(const String& s)
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

template<> ARK_API LayoutParam::FlexWrap StringConvert::eval<LayoutParam::FlexWrap>(const String& s)
{
    if(s == "nowrap")
        return LayoutParam::FLEX_WRAP_NOWRAP;
    if(s == "wrap")
        return LayoutParam::FLEX_WRAP_WRAP;
    CHECK(s == "wrap_reverse", "Unknow enum %s(\"%s\"), possible values are: [%s]", s.c_str(), "FlexWrap", "'nowrap', 'wrap_reverse'");
    return LayoutParam::FLEX_WRAP_WRAP_REVERSE;
}

template<> ARK_API LayoutParam::JustifyContent StringConvert::eval<LayoutParam::JustifyContent>(const String& s)
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
    CHECK(s == "space_evenly", "Unknow enum %s(\"%s\"), possible values are: [%s]", s.c_str(), "JustifyContent", "'flex_start', 'flex_end', 'center', 'space_between', 'space_around', 'space_evenly'");
    return LayoutParam::JUSTIFY_CONTENT_SPACE_EVENLY;
}

template<> ARK_API LayoutParam::Align StringConvert::eval<LayoutParam::Align>(const String& s)
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

LayoutParam::Length::Length(float pixels)
    : Length(LENGTH_TYPE_PIXEL, pixels)
{
}

LayoutParam::Length::Length(LengthType type, float value)
    : _type(type), _value(sp<Numeric>::make<Numeric::Const>(value))
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
