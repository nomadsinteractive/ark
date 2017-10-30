#ifndef ARK_APP_BASE_SURFACE_CONTROLLER_H_
#define ARK_APP_BASE_SURFACE_CONTROLLER_H_

#include "core/base/api.h"
#include "core/base/object_pool.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API SurfaceController {
public:
    SurfaceController();

// [[script::bindings::auto]]
    void addRenderer(const sp<Renderer>& renderer);
// [[script::bindings::auto]]
    void addControl(const sp<Renderer>& control);
// [[script::bindings::auto]]
    void addLayer(const sp<Layer>& layer);

    void requestUpdate();

    sp<RenderCommandPipeline> getRenderCommand();

private:
    sp<RendererGroup> _renderers;
    sp<RendererGroup> _controls;
    sp<RendererGroup> _layers;

    ObjectPool<RenderCommandPipeline> _render_command_pipe_line_pool;
    OCSQueue<sp<RenderCommandPipeline>> _render_commands;
    sp<RenderCommandPipeline> _last_render_command;
    uint32_t _queue_length;
};

}

#endif
