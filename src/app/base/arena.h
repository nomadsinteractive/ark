#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Arena {
public:
    Arena(sp<RenderGroup> renderPhrases, sp<ResourceLoader> resourceLoader, sp<Scope> args);

//  [[script::bindings::auto]]
    const sp<BoxBundle>& layers() const;
//  [[script::bindings::property]]
    const sp<BoxBundle>& renderLayers() const;

private:
    struct Stub;

    struct RenderLayerBundle;

private:
    sp<Stub> _stub;

    sp<BoxBundle> _layers;
    sp<BoxBundle> _render_layers;
};

}
