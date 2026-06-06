#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GraphicsContext {
public:
    GraphicsContext();
    GraphicsContext(sp<RenderController> renderController);

    static GraphicsContext& mocked();

    void onSurfaceReady();
    void onDrawFrame();

    const sp<RenderBackendInfo>& renderBackendInfo() const;
    const sp<RenderController>& renderController() const;
    const sp<Recycler>& recycler() const;

    Traits& traits();

    uint32_t tick() const;

private:
    sp<RenderController> _render_controller;
};

}
