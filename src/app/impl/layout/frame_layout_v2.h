#ifndef ARK_APP_IMPL_LAYOUT_FRAME_LAYOUT_V2_H_
#define ARK_APP_IMPL_LAYOUT_FRAME_LAYOUT_V2_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"

namespace ark {

class FrameLayoutV2 : public LayoutV2 {
public:

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) override;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) override;

//  [[plugin::builder::by-value("frame")]]
    class BUILDER : public Builder<LayoutV2> {
    public:
        BUILDER() = default;

        virtual sp<LayoutV2> build(const Scope& args) override;
    };
};

}

#endif
