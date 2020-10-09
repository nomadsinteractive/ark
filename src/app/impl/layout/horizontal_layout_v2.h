#ifndef ARK_APP_IMPL_LAYOUT_HORIZONTAL_LAYOUT_V2_H_
#define ARK_APP_IMPL_LAYOUT_HORIZONTAL_LAYOUT_V2_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"

namespace ark {

class HorizontalLayoutV2 : public LayoutV2 {
public:

    virtual V2 inflate(const std::vector<sp<Slot>>& children) override;

    virtual std::vector<V2> place(const std::vector<sp<Slot>>& children, const sp<Slot>& parent) override;

//  [[plugin::builder::by-value("horizontal")]]
    class BUILDER : public Builder<LayoutV2> {
    public:
        BUILDER() = default;

        virtual sp<LayoutV2> build(const Scope& args) override;

    };

};

}

#endif
