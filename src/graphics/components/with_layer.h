#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API WithLayer final : public Wirable {
public:
//  [[script::bindings::auto]]
    WithLayer(sp<Layer> layer);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[script::bindings::property]]
    const sp<Layer>& layer() const;

//  [[plugin::builder("with-layer")]]
    class BUILDER final : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        builder<Layer> _layer;
    };

private:
    sp<Layer> _layer;
};

}
