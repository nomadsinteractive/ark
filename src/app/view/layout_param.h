#ifndef ARK_APP_VIEW_LAYOUT_PARAM_H_
#define ARK_APP_VIEW_LAYOUT_PARAM_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

namespace ark {

class LayoutParam {
public:
    enum Display {
        DISPLAY_BLOCK,
        DISPLAY_FLOAT,
        DISPLAY_ABSOLUTE
    };

    enum Gravity {
        NONE = 0,
        LEFT = 1,
        RIGHT = 2,
        CENTER_HORIZONTAL = 3,
        TOP = 4,
        BOTTOM = 8,
        CENTER_VERTICAL = 12,
        CENTER = CENTER_VERTICAL | CENTER_HORIZONTAL,
        GRAVITY_DEFAULT = CENTER
    };

    static const int32_t MATCH_PARENT;
    static const int32_t WRAP_CONTENT;

public:
    LayoutParam(const sp<Size>& size, Display display = DISPLAY_BLOCK, Gravity gravity = NONE);
    LayoutParam(const LayoutParam& other) = default;

    float calcLayoutWidth(float available);
    float calcLayoutHeight(float available);

    float contentWidth() const;
    void setContentWidth(float contentWidth);
    float offsetWidth() const;

    float contentHeight() const;
    void setContentHeight(float contentHeight);
    float offsetHeight() const;

    const SafePtr<Size>& size() const;
    void setSize(const sp<Size>& size);

    const sp<Boolean>& stopPropagation() const;
    void setStopPropagation(sp<Boolean> stopPropagation);

    Display display() const;
    void setDisplay(Display display);

    Gravity gravity() const;
    void setGravity(Gravity gravity);

    const Rect& margins() const;
    Rect& margins();

    bool isWrapContent() const;

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
    sp<Boolean> _stop_propagation;

    Display _display;
    Gravity _gravity;
    Rect _margins;
};

}

#endif
