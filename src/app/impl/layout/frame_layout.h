#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"

namespace ark {

class FrameLayout : public LayoutV3 {
public:

    virtual bool update(uint64_t timestamp) override;
    virtual void inflate(sp<Node> rootNode) override;

//  [[plugin::builder::by-value("frame")]]
    class BUILDER : public Builder<LayoutV3> {
    public:
        BUILDER() = default;

        virtual sp<LayoutV3> build(const Scope& args) override;
    };

private:
    sp<Node> _root_node;
};

}
