#ifndef ARK_APP_IMPL_LAYOUT_HORIZONTAL_LAYOUT_V2_H_
#define ARK_APP_IMPL_LAYOUT_HORIZONTAL_LAYOUT_V2_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"

namespace ark {

class HorizontalLayoutV2 : public LayoutV2 {
public:
    HorizontalLayoutV2(LayoutParam::Gravity layoutGravity);

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) override;

    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& children, const LayoutParam& parent, const V2& contentSize) override;

//  [[plugin::builder::by-value("horizontal")]]
    class BUILDER : public Builder<LayoutV2> {
    public:
        BUILDER(const String& gravity);

        virtual sp<LayoutV2> build(const Scope& args) override;

    private:
        LayoutParam::Gravity _layout_gravity;
    };

private:
    LayoutParam::Gravity _layout_gravity;

};

}

#endif
