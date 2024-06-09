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
    WithLayer(const sp<Layer>& layer);
//  [[script::bindings::auto]]
    WithLayer(const sp<RenderLayer>& renderLayer);
//  [[script::bindings::auto]]
    WithLayer(sp<LayerContext> layerContext);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[script::bindings::property]]
    const sp<LayerContext>& layerContext() const;

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

//  [[plugin::builder("with_layer")]]
    class BUILDER : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        sp<Builder<LayerContext>> _layer_context;
    };

private:
    sp<LayerContext> _layer_context;
};

}
