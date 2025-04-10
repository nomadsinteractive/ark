#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Arena final : public Wirable::Niche {
public:
    Arena(sp<RenderGroup> renderGroup, sp<ResourceLoader> resourceLoader);

//  [[script::bindings::property]]
    const sp<BoxBundle>& layers() const;
//  [[script::bindings::property]]
    const sp<BoxBundle>& renderLayers() const;

    void onPoll(Wirable::WiringContext& context, StringView value) override;

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
