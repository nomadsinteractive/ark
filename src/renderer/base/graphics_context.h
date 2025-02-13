#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GraphicsContext {
public:
    GraphicsContext() = default;
    GraphicsContext(sp<RenderEngineContext> renderContext, sp<RenderController> renderController);

    static GraphicsContext& mocked();

    void onSurfaceReady();
    void onDrawFrame();

    const sp<RenderEngineContext>& renderContext() const;
    const sp<RenderController>& renderController() const;
    const sp<Recycler>& recycler() const;

    Traits& traits();
    const Traits& traits() const;

    uint32_t tick() const;

private:
    sp<RenderEngineContext> _render_context;
    sp<RenderController> _render_controller;
    uint32_t _tick;
};

}
