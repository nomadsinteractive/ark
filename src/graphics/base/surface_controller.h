#pragma once

#include "core/base/api.h"
#include "core/base/allocator.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

class ARK_API SurfaceController {
public:
    SurfaceController();

// [[script::bindings::auto]]
    void addRenderer(sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr, RendererType::Priority priority = RendererType::PRIORITY_DEFAULT);

    void requestUpdate(uint64_t timestamp);
    void onRenderFrame(V4 backgroundColor, RenderView& renderView) const;

private:
    sp<Allocator::Pool> _allocator_pool;
    sp<OCSQueue<RenderRequest>> _render_requests;
    RenderGroup _renderer_phrase;
};

}
