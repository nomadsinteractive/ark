#ifndef ARK_APP_IMPL_LAYOUT_FRAME_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_FRAME_LAYOUT_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class FrameLayout : public Layout {
public:

    virtual void begin(LayoutParam& layoutParam) override;
    virtual Rect place(LayoutParam& layoutParam) override;
    virtual Rect end() override;

//  [[plugin::builder("frame")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER();

        virtual sp<Layout> build(const sp<Scope>& args) override;

    };

};

}

#endif
