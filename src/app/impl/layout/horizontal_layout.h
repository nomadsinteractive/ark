#pragma once

#include "app/inf/layout.h"
#include "app/traits/layout_param.h"

namespace ark {

class HorizontalLayout : public Layout {
public:
    HorizontalLayout(LayoutParam::Align alignItems);

    bool update(uint64_t timestamp) override;
    void inflate(sp<Node> rootNode) override;

//  [[plugin::builder::by-value("horizontal")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(const String& flexDirection);

        sp<Layout> build(const Scope& args) override;

    private:
        LayoutParam::Align _align_items;
    };

private:
    sp<Node> _root_node;
    LayoutParam::Align _align_items;
};

}
