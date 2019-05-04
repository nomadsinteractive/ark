#ifndef ARK_BASE_APP_APPLICATION_FACADE_H_
#define ARK_BASE_APP_APPLICATION_FACADE_H_

#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationFacade {
public:
    ApplicationFacade(Application& app, const Surface& surface, const sp<Manifest>& manifest);

//  [[script::bindings::property]]
    const sp<Clock>& clock() const;
// [[script::bindings::property]]
    const sp<ApplicationController>& controller() const;
// [[script::bindings::property]]
    const sp<Camera>& camera() const;
// [[script::bindings::property]]
    const sp<Manifest>& manifest() const;

// [[script::bindings::property]]
    const sp<Arena>& arena() const;
// [[script::bindings::property]]
    void setArena(const sp<Arena>& arena);

//  [[script::bindings::auto]]
    sp<ResourceLoader> createResourceLoader(const String& name, const sp<Scope>& args = nullptr);

//  [[script::bindings::property]]
    const std::vector<String>& argv() const;

//  [[script::bindings::auto]]
    void addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& expired = nullptr);
//  [[script::bindings::auto]]
    void addEventListener(const sp<EventListener>& eventListener);
//  [[script::bindings::auto]]
    void setDefaultEventListener(const sp<EventListener>& eventListener);

//  [[script::bindings::auto]]
    void exit();

//  [[script::bindings::auto]]
    void post(const sp<Runnable>& task, float delay = 0.0f);
//  [[script::bindings::auto]]
    void schedule(const sp<Runnable>& task, float interval);

//  [[script::bindings::auto]]
    void addStringBundle(const String& name, const sp<StringBundle>& stringBundle);
//  [[script::bindings::auto]]
    sp<String> getString(const String& resid);

//  [[script::bindings::auto]]
    sp<Runnable> defer(const sp<Runnable>& task) const;
//  [[script::bindings::auto]]
    sp<Numeric> synchronize(const sp<Numeric>& value) const;
//  [[script::bindings::auto]]
    sp<Vec2> synchronize(const sp<Vec2>& value) const;
//  [[script::bindings::auto]]
    sp<Vec3> synchronize(const sp<Vec3>& value) const;
//  [[script::bindings::auto]]
    sp<Vec4> synchronize(const sp<Vec4>& value) const;

//  [[script::bindings::property]]
    const Color& backgroundColor() const;
//  [[script::bindings::property]]
    void setBackgroundColor(const Color& backgroundColor);

private:
    sp<ApplicationContext> _context;
    sp<ApplicationController> _controller;
    sp<SurfaceController> _surface_controller;

    sp<Manifest> _manifest;

    sp<Arena> _arena;
};

}

#endif
