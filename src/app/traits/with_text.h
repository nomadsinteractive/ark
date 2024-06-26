#pragma once

#include "core/inf/wirable.h"

#include "graphics/base/text.h"

namespace ark {

class ARK_API WithText final : public Wirable {
public:
//  [[script::bindings::auto]]
    WithText(sp<Text> text, String transformNode = "");

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[plugin::builder("with_text")]]
    class BUILDER final : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        sp<Builder<Text>> _text;
        String _transform_node;
    };

private:
    sp<Text> _text;
    String _transform_node;
};

}
