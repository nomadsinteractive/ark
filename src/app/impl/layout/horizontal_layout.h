#pragma once

#include "graphics/components/layout_param.h"

#include "graphics/inf/layout.h"

namespace ark {

class HorizontalLayout : public Layout {
public:
    HorizontalLayout(LayoutParam::Align alignItems);

    sp<Updatable> inflate(Hierarchy hierarchy) override;

    //  [[plugin::builder::by-value("horizontal")]]
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
