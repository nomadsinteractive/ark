#ifndef ARK_APP_IMPL_LAYOUT_GRAVITY_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_GRAVITY_LAYOUT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class GravityLayout : public Layout {
public:
    GravityLayout(const LayoutParam::Gravity& gravity);

    static Rect place(LayoutParam::Gravity gravity, float clientWidth, float clientHeight, float width, float height);
    static float place(LayoutParam::Gravity gravity, float size, float available);

    virtual void begin(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect end(Context& ctx) override;

//  [[plugin::builder("gravity")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Layout> build(const Scope& args) override;

    private:
        LayoutParam::Gravity _gravity;

    };

private:
    LayoutParam::Gravity _gravity;

};

}

#endif
