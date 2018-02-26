#ifndef ARK_APP_BASE_SURFACE_UPDATER_H_
#define ARK_APP_BASE_SURFACE_UPDATER_H_

#include <list>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class SurfaceUpdater {
public:
    SurfaceUpdater(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController);

    void requestUpdate();

    RenderRequest obtainRenderRequest();

private:
    sp<Executor> _executor;
    sp<SurfaceController> _surface_controller;
    sp<LockFreeStack<RenderRequest>> _render_request_recycler;
};

}

#endif
