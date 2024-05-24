#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/layout_param.h"

namespace ark {

class VerticalLayout : public LayoutV3 {
public:
    VerticalLayout(LayoutParam::FlexDirection flexDirection);

    virtual bool update(uint64_t timestamp) override;
    virtual void inflate(sp<Node> rootNode) override;

//  [[plugin::builder::by-value("vertical")]]
    class BUILDER : public Builder<LayoutV3> {
    public:
        BUILDER(const String& flexDirection);

        virtual sp<LayoutV3> build(const Scope& args) override;

    private:
        LayoutParam::FlexDirection _flex_direction;
    };

private:
    float calcChildNodesTotalHeight() const;

private:
    sp<Node> _root_node;
    LayoutParam::FlexDirection _flex_direction;
};

}
