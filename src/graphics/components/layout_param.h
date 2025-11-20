#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/optional_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/layout_length.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API LayoutParam final : public Updatable {
public:
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

public:
//  [[script::bindings::auto]]
    LayoutParam(LayoutLength width = {}, LayoutLength height = {}, sp<Layout> layout = nullptr, LayoutParam::FlexDirection flexDirection = LayoutParam::FLEX_DIRECTION_ROW, LayoutParam::FlexWrap flexWrap = LayoutParam::FLEX_WRAP_NOWRAP,
                LayoutParam::JustifyContent justifyContent = LayoutParam::JUSTIFY_CONTENT_FLEX_START, LayoutParam::Align alignItems = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Align alignSelf = LayoutParam::ALIGN_AUTO, LayoutParam::Align alignContent = LayoutParam::ALIGN_STRETCH,
                float flexGrow = 0, LayoutLength flexBasis = {}, sp<Vec4> margins = nullptr, sp<Vec4> paddings = nullptr, sp<Vec3> offset = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LayoutParam);

    bool update(uint32_t tick) override;

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
    const LayoutLength& flexBasis() const;
//  [[script::bindings::property]]
    void setFlexBasis(LayoutLength flexBasis);

//  [[script::bindings::property]]
    float flexGrow() const;
//  [[script::bindings::property]]
    void setFlexGrow(float weight);
    bool hasFlexGrow() const;

//  [[script::bindings::property]]
    const LayoutLength& width() const;
//  [[script::bindings::property]]
    void setWidth(LayoutLength width);

//  [[script::bindings::property]]
    const LayoutLength& height() const;
//  [[script::bindings::property]]
    void setHeight(LayoutLength height);

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
    const OptionalVar<Vec4>& margins() const;
//  [[script::bindings::property]]
    void setMargins(sp<Vec4> margins);

//  [[script::bindings::property]]
    const OptionalVar<Vec4>& paddings() const;
//  [[script::bindings::property]]
    void setPaddings(sp<Vec4> paddings);

//  [[script::bindings::property]]
    const OptionalVar<Vec3>& offset() const;
//  [[script::bindings::property]]
    void setOffset(sp<Vec3> offset);

    bool isWrapContent() const;

    const Timestamp& timestamp() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<LayoutParam> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<LayoutParam> build(const Scope& args) override;

    private:
        sp<IBuilder<LayoutLength>> _width;
        sp<IBuilder<LayoutLength>> _height;
        SafeBuilder<Layout> _layout;
        FlexDirection _flex_direction;
        FlexWrap _flex_wrap;
        JustifyContent _justify_content;

        Align _align_items;
        Align _align_self;
        Align _align_content;
        sp<Builder<Size>> _size;
        float _flex_grow;

        SafeBuilder<Vec4> _margins;
        SafeBuilder<Vec4> _paddings;
        SafeBuilder<Vec3> _offset;
    };

private:
    LayoutLength _width;
    LayoutLength _height;
    sp<Layout> _layout;

    FlexDirection _flex_direction;
    FlexWrap _flex_wrap;
    JustifyContent _justify_content;

    Align _align_items;
    Align _align_self;
    Align _align_content;

    sp<Boolean> _stop_propagation;

    LayoutLength _flex_basis;
    float _flex_grow;
    OptionalVar<Vec4> _margins;
    OptionalVar<Vec4> _paddings;
    OptionalVar<Vec3> _offset;

    Timestamp _timestamp;
};

}
