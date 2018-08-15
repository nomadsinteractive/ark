#ifndef ARK_BASE_APP_APPLICATION_FACADE_H_
#define ARK_BASE_APP_APPLICATION_FACADE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationFacade {
public:
    ApplicationFacade(Application& app, const Surface& surface, const sp<ApplicationManifest>& manifest);

//  [[script::bindings::property]]
    const sp<Clock>& clock() const;
// [[script::bindings::property]]
    const sp<ApplicationContext>& context() const;
// [[script::bindings::property]]
    const sp<ApplicationController>& controller() const;
// [[script::bindings::property]]
    const sp<Camera>& camera() const;
// [[script::bindings::property]]
    const sp<ApplicationManifest>& manifest() const;

// [[script::bindings::property]]
    const sp<Arena>& arena() const;
// [[script::bindings::property]]
    void setArena(const sp<Arena>& arena);

// [[script::bindings::property]]
    const Color& backgroundColor() const;
// [[script::bindings::property]]
    void setBackgroundColor(const Color& backgroundColor);

private:
    sp<ApplicationContext> _context;
    sp<ApplicationController> _controller;
    sp<SurfaceController> _surface_controller;

    sp<ApplicationManifest> _manifest;

    sp<Arena> _arena;
};

}

#endif
