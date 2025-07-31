#pragma once

#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class SurfaceUpdater final : public Runnable {
public:
    SurfaceUpdater(sp<SurfaceController> surfaceController, const sp<ApplicationContext>& applicationContext);

    void run() override;

private:
    sp<SurfaceController> _surface_controller;
    sp<ApplicationContext> _application_context;
};

}
