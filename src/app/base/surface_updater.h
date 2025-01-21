#pragma once

#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class SurfaceUpdater final : public Runnable {
public:
    SurfaceUpdater(sp<SurfaceController> surfaceController, const ApplicationContext& applicationContext);

    void run() override;

private:
    sp<SurfaceController> _surface_controller;
    sp<RenderController> _render_controller;
    sp<Variable<uint64_t>::Impl> _app_clock_ticker;
    sp<Numeric::Impl> _app_clock_interval;
    uint64_t _pre_frame_timestamp;
};

}
