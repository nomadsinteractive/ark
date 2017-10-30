#ifndef ARK_APP_BASE_APPLICATION_DELEGATE_H_
#define ARK_APP_BASE_APPLICATION_DELEGATE_H_

#include "core/base/api.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"

#include "app/inf/event_listener.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationDelegate : public EventListener {
public:
    ApplicationDelegate(const Color& backgroundColor);
    virtual ~ApplicationDelegate();

    virtual const char* name() = 0;

    virtual void onCreate(Application& application, const sp<Surface>& surface);
    virtual void onPause();
    virtual void onResume();
    virtual void onDestroy();

    virtual void onSurfaceCreated(const sp<Surface>& surface);
    virtual void onSurfaceChanged(uint32_t width, uint32_t height);
    virtual void onSurfaceDraw();

    virtual bool onEvent(const Event& event);

protected:
    Color _background_color;

    sp<ApplicationContext> _application_context;
    sp<RenderView> _render_view;
    sp<Surface> _surface;
};

}

#endif
