#pragma once

#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Label final : public Wirable {
public:
//  [[script::bindings::auto]]
    Label(sp<Text> text, sp<LayoutParam> layoutParam = nullptr);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[plugin::builder("label")]]
    class BUILDER : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        sp<Builder<Text>> _text;
    };

private:
    sp<Text> _text;
    sp<LayoutParam> _layout_param;
};

}
