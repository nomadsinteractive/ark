#ifndef ARK_APP_BASE_SURFACE_UPDATER_H_
#define ARK_APP_BASE_SURFACE_UPDATER_H_

#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class SurfaceUpdater : public Runnable {
public:
    SurfaceUpdater(const sp<SurfaceController>& surfaceController, const sp<RenderController>& renderController);

    virtual void run() override;

private:
    sp<SurfaceController> _surface_controller;
    sp<RenderController> _render_controller;
    sp<Variable<uint64_t>> _clock;
};

}

#endif
