#ifndef ARK_APP_VIEW_LAYOUT_PARAM_H_
#define ARK_APP_VIEW_LAYOUT_PARAM_H_

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/types/optional.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API LayoutParam : public Updatable {
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

//  [[script::bindings::enumeration]]
    enum LengthType {
        LENGTH_TYPE_AUTO,
        LENGTH_TYPE_PIXEL,
        LENGTH_TYPE_PERCENTAGE
    };

    struct Length {
        Length();
        Length(LengthType type, float value);
        Length(LengthType type, sp<Numeric> value);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Length);

        bool update(uint64_t timestamp) const;

        LengthType _type;
        SafeVar<Numeric> _value;
    };

public:
//  [[script::bindings::auto]]
    LayoutParam(const sp<Size>& size, LayoutParam::Display display = LayoutParam::DISPLAY_BLOCK, LayoutParam::Gravity gravity = LayoutParam::GRAVITY_DEFAULT, float grow = 0);
    LayoutParam(Length width, Length height, LayoutParam::FlexDirection flexDirection = LayoutParam::FLEX_DIRECTION_ROW, LayoutParam::FlexWrap flexWrap = LayoutParam::FLEX_WRAP_NOWRAP,
                LayoutParam::JustifyContent justifyContent = LayoutParam::JUSTIFY_CONTENT_FLEX_START, LayoutParam::Align alignItems = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Align alignSelf = LayoutParam::ALIGN_AUTO, LayoutParam::Align alignContent = LayoutParam::ALIGN_STRETCH,
                LayoutParam::Display display = LayoutParam::DISPLAY_BLOCK, float flexGrow = 0, Length flexBasis = Length(), sp<Vec4> margins = nullptr, sp<Vec4> paddings = nullptr,
                sp<Vec3> position = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LayoutParam);

    virtual bool update(uint64_t timestamp) override;

    float calcLayoutWidth(float available) const;
    float calcLayoutHeight(float available) const;

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
    const sp<Size>& size() const;

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
    LayoutParam::LengthType flexBasisType() const;
//  [[script::bindings::property]]
    void setFlexBasisType(LayoutParam::LengthType basisType);
//  [[script::bindings::property]]
    sp<Numeric> flexBasis() const;
//  [[script::bindings::property]]
    void setFlexBasis(sp<Numeric> flexBasis);

//  [[script::bindings::property]]
    float flexGrow() const;
//  [[script::bindings::property]]
    void setFlexGrow(float weight);

    bool hasFlexGrow() const;

//  [[script::bindings::property]]
    sp<Numeric> width() const;
//  [[script::bindings::property]]
    void setWidth(sp<Numeric> width);

//  [[script::bindings::property]]
    LayoutParam::LengthType widthType() const;
//  [[script::bindings::property]]
    void setWidthType(LayoutParam::LengthType widthType);

//  [[script::bindings::property]]
    sp<Numeric> height() const;
//  [[script::bindings::property]]
    void setHeight(sp<Numeric> height);

//  [[script::bindings::property]]
    LayoutParam::LengthType heightType() const;
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

    const SafeVar<Vec3>& position() const;
    void setPosition(sp<Vec3> position);

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
        sp<Builder<Length>> _width;
        sp<Builder<Length>> _height;
        FlexDirection _flex_direction;
        FlexWrap _flex_wrap;
        JustifyContent _justify_content;

        Align _align_items;
        Align _align_self;
        Align _align_content;
        sp<Builder<Size>> _size;
        Display _display;
        float _flex_grow;

        SafePtr<Builder<Vec4>> _margins;
        SafePtr<Builder<Vec4>> _paddings;
        SafePtr<Builder<Vec3>> _position;
    };

private:
    LengthType _width_type;
    LengthType _height_type;
    sp<Size> _size;

    SafeVar<Size> _size_min;
    SafeVar<Size> _size_max;

    FlexDirection _flex_direction;
    FlexWrap _flex_wrap;
    JustifyContent _justify_content;

    Align _align_items;
    Align _align_self;
    Align _align_content;

    sp<Boolean> _stop_propagation;

    Display _display;
    Gravity _gravity;

    Length _flex_basis;
    float _flex_grow;
    SafeVar<Vec4> _margins;
    SafeVar<Vec4> _paddings;

    SafeVar<Vec3> _position;

    Timestamp _timestamp;
};

}

#endif
