#pragma once

#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/base/application_context.h"

namespace ark {

class SurfaceUpdater final : public Runnable {
public:
    SurfaceUpdater(sp<SurfaceController> surfaceController, const ApplicationContext& applicationContext);

    void run() override;

private:
    sp<SurfaceController> _surface_controller;
    sp<RenderController> _render_controller;
    sp<ApplicationContext::AppClock> _app_clock;
};

}
