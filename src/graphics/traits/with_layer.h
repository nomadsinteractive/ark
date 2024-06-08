#pragma once

#include "core/base/api.h"
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

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[script::bindings::property]]
    const sp<LayerContext>& layerContext() const;

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

private:
    sp<LayerContext> _layer_context;
};

}
