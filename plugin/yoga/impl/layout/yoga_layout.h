#pragma once

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/layout.h"

namespace ark::plugin::yoga {

class YogaLayout final : public Layout {
public:

    sp<Updatable> inflate(Hierarchy hierarchy) override;
    bool removeNode(Node& node) override;
    bool appendNode(Node& parentNode, View& childView) override;

    //  [[plugin::builder::by-value("yoga")]]
    class BUILDER final : public Builder<Layout> {
    public:
        BUILDER() = default;

        sp<Layout> build(const Scope& args) override;
    };

};

}
