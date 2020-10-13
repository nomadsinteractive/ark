#ifndef ARK_APP_IMPL_LAYOUT_FRAME_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_FRAME_LAYOUT_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class FrameLayout : public Layout {
public:

    virtual void begin(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect end(Context& ctx) override;

//  [[plugin::builder("frame")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER() = default;

        virtual sp<Layout> build(const Scope& args) override;

    };

//  [[plugin::builder::by-value("frame")]]
    class DICTIONARY : public Builder<Layout> {
    public:
        DICTIONARY() = default;

        virtual sp<Layout> build(const Scope& args) override;

    };

};

class FrameLayoutNew : public LayoutV2 {
public:
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent) override;

};

}

#endif
