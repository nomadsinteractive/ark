#include "graphics/components/layout_param.h"

#include "core/base/constants.h"
#include "core/base/bean_factory.h"
#include "core/base/enum.h"
#include "core/inf/variable.h"
#include "core/util/documents.h"
#include "core/util/string_convert.h"
#include "core/util/updatable_util.h"

#include "graphics/components/size.h"

namespace ark {

LayoutParam::LayoutParam(LayoutLength width, LayoutLength height, sp<Layout> layout, FlexDirection flexDirection, FlexWrap flexWrap, JustifyContent justifyContent, Align alignItems, Align alignSelf,
                         Align alignContent, float flexGrow, LayoutLength flexBasis, sp<Vec4> margins, sp<Vec4> paddings, sp<Vec3> offset)
    : _width(std::move(width)), _height(std::move(height)), _layout(std::move(layout)), _flex_direction(flexDirection), _flex_wrap(flexWrap), _justify_content(justifyContent), _align_items(alignItems), _align_self(alignSelf), _align_content(alignContent),
      _flex_basis(std::move(flexBasis)), _flex_grow(flexGrow), _margins(std::move(margins)), _paddings(std::move(paddings)), _offset(std::move(offset))
{
}

bool LayoutParam::update(const uint64_t timestamp)
{
    const bool dirty = _timestamp.update(timestamp);
    return UpdatableUtil::update(timestamp, _width, _height, _margins, _paddings, _flex_basis) || dirty;
}

const sp<Layout>& LayoutParam::layout() const
{
    return _layout;
}

void LayoutParam::setLayout(sp<Layout> layout)
{
    _layout = std::move(layout);
}

float LayoutParam::calcLayoutWidth(const float available)
{
    const V4 margins = _margins.val();
    if(_width.isMatchParent())
    {
        _width._value.reset(sp<Numeric>::make<Numeric::Const>(available - margins.w() - margins.y()));
        return available;
    }
    return _width._value.val() + margins.w() + margins.y();
}

float LayoutParam::calcLayoutHeight(float available)
{
    const V4 margins = _margins.val();
    if(_height.isMatchParent())
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

const LayoutLength& LayoutParam::flexBasis() const
{
    return _flex_basis;
}

void LayoutParam::setFlexBasis(LayoutLength flexBasis)
{
    _flex_basis = std::move(flexBasis);
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

const LayoutLength& LayoutParam::width() const
{
    return _width;
}

void LayoutParam::setWidth(LayoutLength width)
{
    _width = std::move(width);
    _timestamp.markDirty();
}

const LayoutLength& LayoutParam::height() const
{
    return _height;
}

void LayoutParam::setHeight(LayoutLength height)
{
    _height = std::move(height);
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

const OptionalVar<Vec4>& LayoutParam::margins() const
{
    return _margins;
}

void LayoutParam::setMargins(sp<Vec4> margins)
{
    _margins = std::move(margins);
}

const OptionalVar<Vec4>& LayoutParam::paddings() const
{
    return _paddings;
}

void LayoutParam::setPaddings(sp<Vec4> paddings)
{
    _paddings.reset(std::move(paddings));
}

const OptionalVar<Vec3>& LayoutParam::offset() const
{
    return _offset;
}

void LayoutParam::setOffset(sp<Vec3> offset)
{
    _offset.reset(std::move(offset));
}

bool LayoutParam::isWrapContent() const
{
    return _flex_wrap == FLEX_WRAP_WRAP || _flex_wrap == FLEX_WRAP_WRAP_REVERSE;
}

LayoutParam::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _width(factory.getIBuilder<LayoutLength>(manifest, constants::WIDTH)), _height(factory.getIBuilder<LayoutLength>(manifest, constants::HEIGHT)), _layout(factory.getBuilder<Layout>(manifest, constants::LAYOUT)),
      _flex_direction(Documents::getAttribute<FlexDirection>(manifest, "flex-direction", FLEX_DIRECTION_ROW)), _flex_wrap(Documents::getAttribute<FlexWrap>(manifest, "flex-wrap", FLEX_WRAP_NOWRAP)),
      _justify_content(Documents::getAttribute<JustifyContent>(manifest, "justify-content", JUSTIFY_CONTENT_FLEX_START)), _align_items(Documents::getAttribute<Align>(manifest, "align-items", ALIGN_STRETCH)),
      _align_self(Documents::getAttribute<Align>(manifest, "align-self", ALIGN_AUTO)), _align_content(Documents::getAttribute<Align>(manifest, "align-content", ALIGN_FLEX_START)),
      _size(factory.getBuilder<Size>(manifest, constants::SIZE)), _flex_grow(Documents::getAttribute<float>(manifest, "flex-grow", 0.0)), _margins(factory.getBuilder<Vec4>(manifest, "margins")),
      _paddings(factory.getBuilder<Vec4>(manifest, "paddings")), _offset(factory.getBuilder<Vec3>(manifest, "offset"))
{
}

sp<LayoutParam> LayoutParam::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size ? _size->build(args) : nullptr;
    sp<Vec4> margins = _margins.build(args);
    sp<Vec4> paddings = _paddings.build(args);
    sp<Vec3> offset = _offset.build(args);
    LayoutLength width = size ? LayoutLength(size->width(), LayoutLength::LENGTH_TYPE_PIXEL) : _width ? _width->build(args) : LayoutLength();
    LayoutLength height = size ? LayoutLength(size->height(), LayoutLength::LENGTH_TYPE_PIXEL) : _height ? _height->build(args) : LayoutLength();
    return sp<LayoutParam>::make(std::move(width), std::move(height), _layout.build(args), _flex_direction, _flex_wrap, _justify_content, _align_items, _align_self, _align_content, _flex_grow,
                                 LayoutLength(), std::move(margins), std::move(paddings), std::move(offset));
}

template<> ARK_API LayoutParam::FlexDirection StringConvert::eval<LayoutParam::FlexDirection>(const String& expr)
{
    constexpr enums::LookupTable<LayoutParam::FlexDirection, 4> table = {{
        {"column", LayoutParam::FLEX_DIRECTION_COLUMN},
        {"column-reverse", LayoutParam::FLEX_DIRECTION_COLUMN_REVERSE},
        {"row", LayoutParam::FLEX_DIRECTION_ROW},
        {"row-reverse", LayoutParam::FLEX_DIRECTION_ROW_REVERSE}
    }};
    return enums::lookup(table, expr);
}

template<> ARK_API LayoutParam::FlexWrap StringConvert::eval<LayoutParam::FlexWrap>(const String& expr)
{
    constexpr enums::LookupTable<LayoutParam::FlexWrap, 3> table = {{
        {"nowrap", LayoutParam::FLEX_WRAP_NOWRAP},
        {"wrap", LayoutParam::FLEX_WRAP_WRAP},
        {"wrap-reverse", LayoutParam::FLEX_WRAP_WRAP}
    }};
    return enums::lookup(table, expr);
}

template<> ARK_API LayoutParam::JustifyContent StringConvert::eval<LayoutParam::JustifyContent>(const String& expr)
{
    constexpr enums::LookupTable<LayoutParam::JustifyContent, 6> table = {{
        {"flex-start", LayoutParam::JUSTIFY_CONTENT_FLEX_START},
        {"flex-end", LayoutParam::JUSTIFY_CONTENT_FLEX_END},
        {"center", LayoutParam::JUSTIFY_CONTENT_CENTER},
        {"space-between", LayoutParam::JUSTIFY_CONTENT_SPACE_BETWEEN},
        {"space-around", LayoutParam::JUSTIFY_CONTENT_SPACE_AROUND},
        {"space-evenly", LayoutParam::JUSTIFY_CONTENT_SPACE_EVENLY}
    }};
    return enums::lookup(table, expr);
}

template<> ARK_API LayoutParam::Align StringConvert::eval<LayoutParam::Align>(const String& expr)
{
    constexpr enums::LookupTable<LayoutParam::Align, 8> table = {{
        {"auto", LayoutParam::ALIGN_AUTO},
        {"flex-start", LayoutParam::ALIGN_FLEX_START},
        {"flex-end", LayoutParam::ALIGN_FLEX_END},
        {"center", LayoutParam::ALIGN_CENTER},
        {"stretch", LayoutParam::ALIGN_STRETCH},
        {"baseline", LayoutParam::ALIGN_BASELINE},
        {"space-between", LayoutParam::ALIGN_SPACE_BETWEEN},
        {"space-around", LayoutParam::ALIGN_SPACE_AROUND}
    }};
    return enums::lookup(table, expr);
}

}
