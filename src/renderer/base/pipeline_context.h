#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"

namespace ark {

class PipelineContext {
public:
    sp<PipelineBindings> _bindings;
    sp<RenderLayerSnapshot::BufferObject> _buffer_object;
};

}
