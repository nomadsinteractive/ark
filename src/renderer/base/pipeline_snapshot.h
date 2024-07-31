#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

struct PipelineSnapshot final {
    sp<ShaderBindings> _bindings;
    std::vector<RenderLayerSnapshot::UBOSnapshot> _ubos;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> _ssbos;
};

}
