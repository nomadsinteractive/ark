#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Surface {
public:
    Surface(sp<RenderView> renderView, const sp<ApplicationContext>& applicationContext);

    const sp<RenderView>& renderView() const;
    const sp<SurfaceController>& controller() const;

    void onSurfaceCreated() const;
    void onSurfaceChanged(uint32_t width, uint32_t height) const;
    void onRenderFrame(V4 backgroundColor) const;

private:
    void requestUpdate() const;

private:
    sp<RenderView> _render_view;
    sp<ApplicationContext> _application_context;

    sp<SurfaceController> _surface_controller;
    sp<Runnable> _update_requester;
};

}
