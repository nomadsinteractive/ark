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

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

    std::vector<std::pair<TypeId, Box>> onWire(const Traits& components) override;

private:
    sp<LayerContext> _layer_context;
};

}
