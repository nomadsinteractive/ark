#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/inf/event_listener.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationDelegate : public EventListener {
public:

    virtual void onCreate(Application& application, const sp<Surface>& surface);
    virtual void onPause();
    virtual void onResume();
    virtual void onDestroy();

    virtual void onSurfaceCreated(const sp<Surface>& surface);
    virtual void onSurfaceChanged(uint32_t width, uint32_t height);
    virtual void onSurfaceDraw();

    bool onEvent(const Event& event) override;

protected:
    sp<ApplicationContext> _application_context;
    sp<RenderView> _render_view;
    sp<Surface> _surface;
};

}
