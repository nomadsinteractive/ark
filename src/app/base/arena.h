#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Arena final : public Wirable::Niche {
public:
//  [[script::bindings::auto]]
    Arena(sp<ResourceLoader> resourceLoader, sp<Boolean> discarded = nullptr, sp<Renderer> renderer = nullptr, Map<String, sp<RenderLayer>> renderLayers = {}, Map<String, sp<Layer>> layers = {});
    ~Arena() override;

//  [[script::bindings::auto]]
    void discard() const;

//  [[script::bindings::property]]
    const sp<BoxBundle>& layers() const;
//  [[script::bindings::property]]
    const sp<BoxBundle>& renderLayers() const;

//  [[script::bindings::auto]]
    void addLayer(String name, sp<Layer> layer) const;

//  [[script::bindings::auto]]
    void addRenderLayer(String name, sp<RenderLayer> renderLayer) const;

    void onPoll(Wirable::WiringContext& context, const document& component) override;

private:
    struct Stub;

    class LayerBundle;
    class RenderLayerBundle;

private:
    sp<Stub> _stub;

    sp<BoxBundle> _layers;
    sp<BoxBundle> _render_layers;
};

}
