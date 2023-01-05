#ifndef ARK_APP_VIEW_LAYOUT_PARAM_H_
#define ARK_APP_VIEW_LAYOUT_PARAM_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
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

    enum JustifyContent {
        JUSTIFY_CONTENT_FLEX_START,
        JUSTIFY_CONTENT_FLEX_END,
        JUSTIFY_CONTENT_CENTER,
        JUSTIFY_CONTENT_SPACE_BETWEEN,
        JUSTIFY_CONTENT_SPACE_AROUND,
        JUSTIFY_CONTENT_SPACE_EVENLY
    };

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

    enum FlexDirection {
        FLEX_DIRECTION_NONE,
        FLEX_DIRECTION_COLUMN,
        FLEX_DIRECTION_COLUMN_REVERSE,
        FLEX_DIRECTION_ROW,
        FLEX_DIRECTION_ROW_REVERSE
    };

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

    struct Length {
        Length();
        Length(LengthType type, float value);

        LengthType _type;
        float _value;
    };

public:
//  [[script::bindings::auto]]
    LayoutParam(const sp<Size>& size, LayoutParam::Display display = LayoutParam::DISPLAY_BLOCK, LayoutParam::Gravity gravity = LayoutParam::GRAVITY_DEFAULT, float weight = 0);
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
    float weight() const;
//  [[script::bindings::property]]
    void setWeight(float weight);
    bool hasWeight() const;

    const SafeVar<Vec4>& margins() const;
    void setMargins(sp<Vec4> margins);

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
        sp<Builder<Size>> _size;
        Display _display;
    };

private:
    SafePtr<Size> _size;
    SafeVar<Vec4> _margins;
    sp<Boolean> _stop_propagation;

    Display _display;
    Gravity _gravity;

    float _weight;
};

}

#endif
