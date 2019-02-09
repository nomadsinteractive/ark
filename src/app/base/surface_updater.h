#ifndef ARK_APP_BASE_SURFACE_UPDATER_H_
#define ARK_APP_BASE_SURFACE_UPDATER_H_

#include "core/concurrent/lf_stack.h"
#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class SurfaceUpdater : public Runnable {
public:
    SurfaceUpdater(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, const sp<RenderController>& renderController);

    virtual void run() override;

    void update();

    void requestUpdate();

private:
    RenderRequest obtainRenderRequest();

private:
    sp<Executor> _executor;
    sp<SurfaceController> _surface_controller;
    sp<RenderController> _render_controller;
    sp<LFStack<RenderRequest>> _render_request_recycler;
};

}

#endif
