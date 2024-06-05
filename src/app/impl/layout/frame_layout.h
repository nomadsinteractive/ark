#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "graphics/inf/layout.h"

namespace ark {

class FrameLayout : public Layout {
public:

    sp<Updatable> inflate(Hierarchy hierarchy) override;

//  [[plugin::builder::by-value("frame")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER() = default;

        sp<Layout> build(const Scope& args) override;
    };

private:
    sp<Node> _root_node;
};

}
