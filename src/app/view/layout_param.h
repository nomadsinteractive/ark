#ifndef ARK_APP_VIEW_LAYOUT_PARAM_H_
#define ARK_APP_VIEW_LAYOUT_PARAM_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

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

    static const float MATCH_PARENT;
    static const float WRAP_CONTENT;

public:
    LayoutParam(const sp<Size>& size, Display display = DISPLAY_BLOCK);
    LayoutParam(const LayoutParam& other) = default;

    float calcLayoutWidth(float available);
    float calcLayoutHeight(float available);

    float contentWidth() const;
    void setContentWidth(float contentWidth);

    float contentHeight() const;
    void setContentHeight(float contentHeight);

    const sp<Size>& size() const;
    const void setSize(const sp<Size>& size);

    Display display() const;
    void setDisplay(Display display);

    const Rect& margins() const;
    Rect& margins();

    bool isWrapContent() const;

    static bool isMatchParent(float unit);
    static bool isWrapContent(float unit);

//  [[plugin::builder]]
    class BUILDER : public Builder<LayoutParam> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<LayoutParam> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Size>> _size;
        Display _display;
    };

private:
    sp<Size> _size;

    Display _display;
    Rect _margins;
};

}

#endif
