#ifndef ARK_APP_BASE_SURFACE_H_
#define ARK_APP_BASE_SURFACE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Surface {
public:
    Surface(const sp<RenderView>& renderView);

    const sp<RenderView>& renderView() const;
    const sp<SurfaceController>& controller() const;

    void onSurfaceCreated();
    void onSurfaceChanged(uint32_t width, uint32_t height);
    void onRenderFrame(const Color& backgroundColor);

    void scheduleUpdate(const sp<ApplicationContext>& applicationContext, uint32_t fps);

private:
    sp<SurfaceController> _surface_controller;
    sp<RenderView> _render_view;

};

}

#endif
