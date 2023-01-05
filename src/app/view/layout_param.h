#ifndef ARK_APP_VIEW_LAYOUT_PARAM_H_
#define ARK_APP_VIEW_LAYOUT_PARAM_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/optional.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API LayoutParam {
public:
//  [[script::bindings::enumeration]]
    enum Display {
        DISPLAY_BLOCK,
        DISPLAY_FLOAT,
        DISPLAY_ABSOLUTE
    };

//  [[script::bindings::enumeration]]
    enum Gravity {
        GRAVITY_DEFAULT = 0,
        GRAVITY_LEFT = 1,
        GRAVITY_RIGHT = 2,
        GRAVITY_CENTER_HORIZONTAL = 3,
        GRAVITY_TOP = 4,
        GRAVITY_BOTTOM = 8,
        GRAVITY_CENTER_VERTICAL = 12,
        GRAVITY_CENTER = GRAVITY_CENTER_VERTICAL | GRAVITY_CENTER_HORIZONTAL
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

    enum SizeConstraint {
        SIZE_CONSTRAINT_MATCH_PARENT = -1,
        SIZE_CONSTRAINT_WRAP_CONTENT = -2
    };

    enum LengthType {
        LENGTH_TYPE_AUTO,
        LENGTH_TYPE_PIXEL,
        LENGTH_TYPE_PERCENTAGE
    };

    struct ARK_API Length {
        Length();
        Length(LengthType type, float value);

        bool isAuto() const;
        bool isPixel() const;
        bool isPercentage() const;

        LengthType _type;
        float _value;
    };

public:
//  [[script::bindings::auto]]
    LayoutParam(const sp<Size>& size, LayoutParam::Display display = LayoutParam::DISPLAY_BLOCK, LayoutParam::Gravity gravity = LayoutParam::GRAVITY_DEFAULT, float grow = 0);
    LayoutParam(Length width, Length height, LayoutParam::FlexDirection flexDirection = LayoutParam::FLEX_DIRECTION_ROW, LayoutParam::FlexWrap flexWrap = LayoutParam::FLEX_WRAP_NOWRAP,
                LayoutParam::JustifyContent justifyContent = LayoutParam::JUSTIFY_CONTENT_FLEX_START, LayoutParam::Align alignItems = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Align alignSelf = LayoutParam::ALIGN_AUTO, LayoutParam::Align alignContent = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Display display = LayoutParam::DISPLAY_BLOCK, float flexGrow = 0);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LayoutParam);

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

    float offsetWidth() const;
    float offsetHeight() const;

//  [[script::bindings::property]]
    const SafePtr<Size>& size() const;
//  [[script::bindings::property]]
    void setSize(const sp<Size>& size);

    const sp<Boolean>& stopPropagation() const;
    void setStopPropagation(sp<Boolean> stopPropagation);

//  [[script::bindings::property]]
    LayoutParam::Display display() const;
//  [[script::bindings::property]]
    void setDisplay(LayoutParam::Display display);

//  [[script::bindings::property]]
    LayoutParam::Gravity gravity() const;
//  [[script::bindings::property]]
    void setGravity(LayoutParam::Gravity gravity);

//  [[script::bindings::property]]
    float flexGrow() const;
//  [[script::bindings::property]]
    void setFlexGrow(float weight);

    bool hasFlexGrow() const;

    const Length& width() const;
    const Length& height() const;

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

    const SafeVar<Vec4>& margins() const;
    void setMargins(sp<Vec4> margins);

    const SafeVar<Vec4>& paddings() const;
    void setPaddings(sp<Vec4> paddings);

    bool isWrapContent() const;
    bool isWidthWrapContent() const;
    bool isHeightWrapContent() const;

    bool isMatchParent() const;
    bool isWidthMatchParent() const;
    bool isHeightMatchParent() const;

    static bool isMatchParent(float unit);
    static bool isWrapContent(float unit);

//  [[plugin::builder]]
    class BUILDER : public Builder<LayoutParam> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<LayoutParam> build(const Scope& args) override;

    private:
        Optional<Length> _width;
        Optional<Length> _height;
        sp<Builder<Size>> _size;
        Display _display;
    };

private:
    Length _width;
    Length _height;

    FlexDirection _flex_direction;
    FlexWrap _flex_wrap;
    JustifyContent _justify_content;

    Align _align_items;
    Align _align_self;
    Align _align_content;

    SafePtr<Size> _size;
    SafeVar<Vec4> _margins;
    SafeVar<Vec4> _paddings;
    sp<Boolean> _stop_propagation;

    Display _display;
    Gravity _gravity;

    float _flex_grow;
};

}

#endif
