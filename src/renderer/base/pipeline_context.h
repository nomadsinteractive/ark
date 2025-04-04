#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/base/render_buffer_snapshot.h"

#include "renderer/forwarding.h"

namespace ark {

class PipelineContext {
public:
    sp<PipelineBindings> _bindings;
    sp<RenderBufferSnapshot> _buffer_snapshot;
};

}
