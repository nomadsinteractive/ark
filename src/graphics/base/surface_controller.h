#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/allocator.h"
#include "core/concurrent/lf_queue.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

class ARK_API SurfaceController {
public:
    SurfaceController();

// [[script::bindings::auto]]
    void addRenderer(sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr, RendererType::Priority priority = RendererType::PRIORITY_DEFAULT);

    void requestUpdate(uint32_t tick);
    void onRenderFrame(V4 backgroundColor, RenderView& renderView);

private:
    RenderRequest obtainRenderRequest();

private:
    sp<Allocator::Pool> _allocator_pool;
    LFQueue<RenderRequest> _render_requests;
    RenderGroup _renderer_phrase;
};

}
