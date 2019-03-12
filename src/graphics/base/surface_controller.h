#ifndef ARK_GRAPHICS_BASE_SURFACE_CONTROLLER_H_
#define ARK_GRAPHICS_BASE_SURFACE_CONTROLLER_H_

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
    void addLayer(const sp<RenderLayer>& layer);

    void postRenderCommand(const sp<RenderCommandPipeline>& renderCommand);

    void update(RenderRequest& renderRequest);

    void onRenderFrame(const Color& backgroundColor, RenderView& renderView);

private:
    sp<RendererGroup> _renderers;
    sp<RendererGroup> _controls;
    sp<RendererGroup> _layers;

    ObjectPool _object_pool;
    OCSQueue<sp<RenderCommandPipeline>> _render_commands;
};

}

#endif
