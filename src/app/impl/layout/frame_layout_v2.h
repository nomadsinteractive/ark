#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"

namespace ark {

class FrameLayoutV2 : public Layout {
public:

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) override;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) override;

//  [[plugin::builder::by-value("frame")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER() = default;

        virtual sp<Layout> build(const Scope& args) override;
    };
};

}
