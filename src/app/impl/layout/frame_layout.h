#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"

namespace ark {

class FrameLayout : public Layout {
public:

    virtual bool update(uint64_t timestamp) override;
    virtual void inflate(sp<Node> rootNode) override;

//  [[plugin::builder::by-value("frame")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER() = default;

        virtual sp<Layout> build(const Scope& args) override;
    };

private:
    sp<Node> _root_node;
};

}
