#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"
#include "graphics/components/layout_param.h"

#include "graphics/inf/layout.h"

namespace ark {

class VerticalLayout : public Layout {
public:
    VerticalLayout(LayoutParam::Align alignItems);

    sp<Updatable> inflate(Hierarchy hierarchy) override;

    //  [[plugin::builder::by-value("vertical")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(const String& flexDirection);

        sp<Layout> build(const Scope& args) override;

    private:
        LayoutParam::Align _align_items;
    };

private:
    LayoutParam::Align _align_items;
};

}
