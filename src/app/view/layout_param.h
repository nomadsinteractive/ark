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

public:
    LayoutParam(const sp<Size>& size, Display display = DISPLAY_BLOCK);

    float calcLayoutWidth(float available);
    float calcLayoutHeight(float available);

    float contentWidth() const;
    float contentHeight() const;

    const sp<Size>& size() const;
    const void setSize(const sp<Size>& size);

    static sp<Size> parseSize(BeanFactory& beanFactory, const String& value, const sp<Scope>& args);

    Display display() const;
    void setDisplay(Display display);

    const Rect& margins() const;
    Rect& margins();

//  [[plugin::builder]]
    class BUILDER : public Builder<LayoutParam> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<LayoutParam> build(const sp<Scope>& args) override;

    private:
        BeanFactory _bean_factory;
        String _size;
        Display _display;
    };

private:
    bool isMatchParent(float unit) const;
    bool isWrapContent(float unit) const;

    static const sp<Numeric> getUnit(BeanFactory& beanFactory, const String& value, const sp<Scope>& args);

private:
    sp<Size> _size;

    Display _display;
    Rect _margins;

    float _content_width;
    float _content_height;

};

}

#endif
