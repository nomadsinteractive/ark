#ifndef ARK_APP_IMPL_LAYOUT_GRAVITY_LAYOUT_V2_H_
#define ARK_APP_IMPL_LAYOUT_GRAVITY_LAYOUT_V2_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/inf/layout.h"
#include "app/view/layout_param.h"

namespace ark {

class GravityLayoutV2 : public LayoutV2 {
public:
    GravityLayoutV2(const LayoutParam::Gravity& gravity);

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) override;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) override;

//  [[plugin::builder::by-value("gravity")]]
    class STYLE : public Builder<LayoutV2> {
    public:
        STYLE(const String& gravity);

        virtual sp<LayoutV2> build(const Scope& args) override;

    private:
        LayoutParam::Gravity _gravity;
    };

private:
    LayoutParam::Gravity _gravity;
};

}

#endif
