#ifndef ARK_GRAPHICS_BASE_SURFACE_CONTROLLER_H_
#define ARK_GRAPHICS_BASE_SURFACE_CONTROLLER_H_

#include "core/base/api.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API SurfaceController {
public:
    SurfaceController(const sp<Executor>& executor);

// [[script::bindings::auto]]
    void addRenderer(const sp<Renderer>& renderer);
// [[script::bindings::auto]]
    void addControlLayer(const sp<Renderer>& controller);
// [[script::bindings::auto]]
    void addLayer(const sp<Renderer>& layer);

    void requestUpdate(uint64_t timestamp);

    void onRenderFrame(const Color& backgroundColor, RenderView& renderView);

private:
    sp<Executor> _executor;
    sp<MemoryPool> _memory_pool;

    sp<RendererGroup> _renderers;
    sp<RendererGroup> _controls;
    sp<RendererGroup> _layers;

    sp<OCSQueue<RenderRequest>> _render_requests;
};

}

#endif
