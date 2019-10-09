#ifndef ARK_APP_IMPL_LAYOUT_VERTICAL_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_VERTICAL_LAYOUT_H_

#include "core/inf/builder.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class VerticalLayout : public Layout {
public:
    VerticalLayout(const LayoutParam::Gravity gravity);

    virtual void begin(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect end(Context& ctx) override;

//  [[plugin::builder("vertical")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Layout> build(const Scope& args) override;

    private:
        LayoutParam::Gravity _gravity;

    };

private:
    LayoutParam::Gravity _place_gravity, _end_gravity;
    float _content_available;
    float _flowy;
    float _max_width;
};

}

#endif
