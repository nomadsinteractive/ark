#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/types/safe_builder.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API LayoutParam final : public Updatable {
public:
//  [[script::bindings::enumeration]]
    enum Display {
        DISPLAY_BLOCK,
        DISPLAY_FLOAT,
        DISPLAY_ABSOLUTE
    };

//  [[script::bindings::enumeration]]
    enum JustifyContent {
        JUSTIFY_CONTENT_FLEX_START,
        JUSTIFY_CONTENT_FLEX_END,
        JUSTIFY_CONTENT_CENTER,
        JUSTIFY_CONTENT_SPACE_BETWEEN,
        JUSTIFY_CONTENT_SPACE_AROUND,
        JUSTIFY_CONTENT_SPACE_EVENLY
    };

//  [[script::bindings::enumeration]]
    enum Align {
        ALIGN_AUTO,
        ALIGN_FLEX_START,
        ALIGN_CENTER,
        ALIGN_FLEX_END,
        ALIGN_STRETCH,
        ALIGN_BASELINE,
        ALIGN_SPACE_BETWEEN,
        ALIGN_SPACE_AROUND
    };

//  [[script::bindings::enumeration]]
    enum FlexDirection {
        FLEX_DIRECTION_COLUMN,
        FLEX_DIRECTION_COLUMN_REVERSE,
        FLEX_DIRECTION_ROW,
        FLEX_DIRECTION_ROW_REVERSE
    };

//  [[script::bindings::enumeration]]
    enum FlexWrap {
        FLEX_WRAP_NOWRAP,
        FLEX_WRAP_WRAP,
        FLEX_WRAP_WRAP_REVERSE
    };

//  [[script::bindings::enumeration]]
    enum LengthType {
        LENGTH_TYPE_AUTO,
        LENGTH_TYPE_PIXEL,
        LENGTH_TYPE_PERCENTAGE
    };

    struct Length {
        Length();
        Length(float pixels);
        Length(LengthType type, float value);
        Length(LengthType type, sp<Numeric> value);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Length);

        bool update(uint64_t timestamp) const;

        LengthType _type;
        SafeVar<Numeric> _value;
    };

public:
    LayoutParam(Length width, Length height, sp<Layout> layout = nullptr, LayoutParam::FlexDirection flexDirection = LayoutParam::FLEX_DIRECTION_ROW, LayoutParam::FlexWrap flexWrap = LayoutParam::FLEX_WRAP_NOWRAP,
                LayoutParam::JustifyContent justifyContent = LayoutParam::JUSTIFY_CONTENT_FLEX_START, LayoutParam::Align alignItems = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Align alignSelf = LayoutParam::ALIGN_AUTO, LayoutParam::Align alignContent = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Display display = LayoutParam::DISPLAY_BLOCK, float flexGrow = 0, Length flexBasis = {}, sp<Vec4> margins = nullptr, sp<Vec4> paddings = nullptr,
                sp<Vec3> offset = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LayoutParam);

    bool update(uint64_t timestamp) override;

    const sp<Layout>& layout() const;
    void setLayout(sp<Layout> layout);

    float calcLayoutWidth(float available);
    float calcLayoutHeight(float available);

//  [[script::bindings::property]]
    float contentWidth() const;
//  [[script::bindings::property]]
    void setContentWidth(float contentWidth);

//  [[script::bindings::property]]
    float contentHeight() const;
//  [[script::bindings::property]]
    void setContentHeight(float contentHeight);

    float occupyWidth() const;
    float occupyHeight() const;

    const sp<Boolean>& stopPropagation() const;
    void setStopPropagation(sp<Boolean> stopPropagation);

//  [[script::bindings::property]]
    LayoutParam::Display display() const;
//  [[script::bindings::property]]
    void setDisplay(LayoutParam::Display display);

//  [[script::bindings::property]]
    LayoutParam::LengthType flexBasisType() const;
//  [[script::bindings::property]]
    void setFlexBasisType(LayoutParam::LengthType basisType);
//  [[script::bindings::property]]
    const SafeVar<Numeric>& flexBasis() const;
//  [[script::bindings::property]]
    void setFlexBasis(sp<Numeric> flexBasis);

//  [[script::bindings::property]]
    float flexGrow() const;
//  [[script::bindings::property]]
    void setFlexGrow(float weight);
    bool hasFlexGrow() const;

    const Length& width() const;
    void setWidth(sp<Numeric> width);

    LayoutParam::LengthType widthType() const;
    void setWidthType(LayoutParam::LengthType widthType);

    const Length& height() const;
    void setHeight(sp<Numeric> height);

//  [[script::bindings::property]]
    void setHeightType(LayoutParam::LengthType heightType);

//  [[script::bindings::property]]
    LayoutParam::FlexDirection flexDirection() const;
//  [[script::bindings::property]]
    LayoutParam::FlexWrap flexWrap() const;
//  [[script::bindings::property]]
    LayoutParam::JustifyContent justifyContent() const;
//  [[script::bindings::property]]
    LayoutParam::Align alignItems() const;
//  [[script::bindings::property]]
    LayoutParam::Align alignSelf() const;
//  [[script::bindings::property]]
    LayoutParam::Align alignContent() const;

//  [[script::bindings::property]]
    const SafeVar<Vec4>& margins() const;
//  [[script::bindings::property]]
    void setMargins(sp<Vec4> margins);

//  [[script::bindings::property]]
    const SafeVar<Vec4>& paddings() const;
//  [[script::bindings::property]]
    void setPaddings(sp<Vec4> paddings);

//  [[script::bindings::property]]
    const SafeVar<Vec3>& offset() const;
//  [[script::bindings::property]]
    void setOffset(sp<Vec3> offset);

    bool isWrapContent() const;
    bool isWidthWrapContent() const;
    bool isHeightWrapContent() const;

    bool isMatchParent() const;
    bool isWidthMatchParent() const;
    bool isHeightMatchParent() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<LayoutParam> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<LayoutParam> build(const Scope& args) override;

    private:
        sp<Builder<Length>> _width;
        sp<Builder<Length>> _height;
        SafeBuilder<Layout> _layout;
        FlexDirection _flex_direction;
        FlexWrap _flex_wrap;
        JustifyContent _justify_content;

        Align _align_items;
        Align _align_self;
        Align _align_content;
        sp<Builder<Size>> _size;
        Display _display;
        float _flex_grow;

        SafeBuilder<Vec4> _margins;
        SafeBuilder<Vec4> _paddings;
        SafeBuilder<Vec3> _offset;
    };

private:
    Length _width;
    Length _height;
    sp<Layout> _layout;

    FlexDirection _flex_direction;
    FlexWrap _flex_wrap;
    JustifyContent _justify_content;

    Align _align_items;
    Align _align_self;
    Align _align_content;

    sp<Boolean> _stop_propagation;

    Display _display;

    Length _flex_basis;
    float _flex_grow;
    SafeVar<Vec4> _margins;
    SafeVar<Vec4> _paddings;

    SafeVar<Vec3> _offset;

    Timestamp _timestamp;
};

}
