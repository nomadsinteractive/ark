#ifndef ARK_APP_IMPL_LAYOUT_VERTICAL_LAYOUT_V2_H_
#define ARK_APP_IMPL_LAYOUT_VERTICAL_LAYOUT_V2_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/layout_param.h"

namespace ark {

class VerticalLayoutV2 : public Layout {
public:
    VerticalLayoutV2(LayoutParam::Gravity layoutGravity);

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) override;

    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& children, const LayoutParam& parent, const V2& contentSize) override;

//  [[plugin::builder::by-value("vertical")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(const String& gravity);

        virtual sp<Layout> build(const Scope& args) override;

    private:
        LayoutParam::Gravity _layout_gravity;
    };

private:
    LayoutParam::Gravity _layout_gravity;

};

}

#endif
