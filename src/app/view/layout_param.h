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
        GRAVITY_NONE = 0,
        GRAVITY_LEFT = 1,
        GRAVITY_RIGHT = 2,
        GRAVITY_CENTER_HORIZONTAL = 3,
        GRAVITY_TOP = 4,
        GRAVITY_BOTTOM = 8,
        GRAVITY_CENTER_VERTICAL = 12,
        GRAVITY_CENTER = GRAVITY_CENTER_VERTICAL | GRAVITY_CENTER_HORIZONTAL,
        GRAVITY_DEFAULT = GRAVITY_CENTER
    };

    static const int32_t MATCH_PARENT;
    static const int32_t WRAP_CONTENT;

public:
    LayoutParam(const sp<Size>& size, Display display = DISPLAY_BLOCK, Gravity gravity = GRAVITY_NONE);
    LayoutParam(const LayoutParam& other) = default;

    float calcLayoutWidth(float available);
    float calcLayoutHeight(float available);

//  [[script::bindings::property]]
    float contentWidth() const;
//  [[script::bindings::property]]
    void setContentWidth(float contentWidth);

    float offsetWidth() const;

//  [[script::bindings::property]]
    float contentHeight() const;
//  [[script::bindings::property]]
    void setContentHeight(float contentHeight);
    float offsetHeight() const;

//  [[script::bindings::property]]
    const SafePtr<Size>& size() const;
//  [[script::bindings::property]]
    void setSize(const sp<Size>& size);

    const sp<Boolean>& stopPropagation() const;
    void setStopPropagation(sp<Boolean> stopPropagation);

    Display display() const;
    void setDisplay(Display display);

    Gravity gravity() const;
    void setGravity(Gravity gravity);

    const SafeVar<Vec4>& margins() const;
    void setMargins(sp<Vec4> margins);

    bool isWrapContent() const;
    bool isWidthWrapContent() const;
    bool isHeightWrapContent() const;

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
};

}

#endif
