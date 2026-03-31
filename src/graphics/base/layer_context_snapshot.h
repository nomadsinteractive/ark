#pragma once

#include "graphics/base/v3.h"
#include "graphics/base/render_layer.h"

#include "renderer/components/varyings.h"

namespace ark {

class LayerContextSnapshot {
public:
    const PipelineLayout& pipelineInput() const;
    const sp<ModelLoader>& modelLoader() const;

    bool _dirty;
    V3 _position;
    bool _visible;
    bool _discarded;
    Varyings::Snapshot _varyings;
    sp<RenderLayer::Stub> _render_layer_stub;
};

}
