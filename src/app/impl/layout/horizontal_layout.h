#ifndef ARK_APP_IMPL_LAYOUT_HORIZONTAL_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_HORIZONTAL_LAYOUT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class HorizontalLayout : public Layout {
public:
    HorizontalLayout(const LayoutParam::Gravity& gravity);

    virtual void begin(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect end(Context& ctx) override;

//  [[plugin::builder("horizontal")]]
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
    float _flowx;
    float _max_height;
};

}

#endif
