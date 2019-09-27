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
    GravityLayout(const View::Gravity& gravity);

    static Rect place(View::Gravity gravity, float clientWidth, float clientHeight, float width, float height);

    virtual void begin(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect end(Context& ctx) override;

//  [[plugin::builder("gravity")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Layout> build(const Scope& args) override;

    private:
        View::Gravity _gravity;

    };

private:
    View::Gravity _gravity;

};

}

#endif
