#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationFacade {
public:
    ApplicationFacade(Application& app, const Surface& surface);

//  [[script::bindings::property]]
    const sp<Clock>& clock() const;
//  [[script::bindings::property]]
    sp<Numeric> clockInterval() const;
//  [[script::bindings::property]]
    sp<Vec2> cursorPosition() const;
//  [[script::bindings::auto]]
    sp<Vec2> toFragCoord(sp<Vec2> xy, sp<Size> resolution = nullptr) const;
//  [[script::bindings::property]]
    const sp<Size>& surfaceSize() const;
//  [[script::bindings::property]]
    const sp<ApplicationController>& applicationController() const;
//  [[script::bindings::property]]
    const sp<ApplicationBundle>& applicationBundle() const;
//  [[script::bindings::property]]
    const sp<RenderController>& renderController() const;
//  [[script::bindings::property]]
    const sp<SurfaceController>& surfaceController() const;
//  [[script::bindings::property]]
    sp<Camera> camera() const;
//  [[script::bindings::property]]
    const sp<ApplicationManifest>& manifest() const;
//  [[script::bindings::property]]
    const sp<ResourceLoader>& resourceLoader() const;

//  [[script::bindings::property]]
    const sp<Activity>& activity() const;
//  [[script::bindings::property]]
    void setActivity(sp<Activity> activity);

//  [[script::bindings::auto]]
    sp<ResourceLoader> createResourceLoader(const String& name, const Scope& args);
//  [[script::bindings::auto]]
    sp<MessageLoop> makeMessageLoop(const sp<Clock>& clock);

//  [[script::bindings::property]]
    const Vector<String>& argv() const;

//  [[script::bindings::auto]]
    void addPreRenderTask(sp<Runnable> task, sp<Boolean> cancelled);
//  [[script::bindings::auto]]
    void addEventListener(sp<EventListener> eventListener, sp<Boolean> disposed = nullptr);
//  [[script::bindings::auto]]
    void pushEventListener(sp<EventListener> eventListener, sp<Boolean> disposed = nullptr);
//  [[script::bindings::auto]]
    void setDefaultEventListener(sp<EventListener> eventListener);

//  [[script::bindings::auto]]
    void exit();

//  [[script::bindings::auto]]
    void post(sp<Runnable> task, float delays, sp<Boolean> canceled = nullptr);
//  [[script::bindings::auto]]
    void post(sp<Runnable> task, const Vector<float>& delays, const sp<Boolean>& canceled = nullptr);
//  [[script::bindings::auto]]
    void schedule(sp<Runnable> task, float interval, sp<Boolean> canceled = nullptr);

//  [[script::bindings::auto]]
    sp<Future> expect(sp<Boolean> condition, sp<Runnable> observer, sp<Boolean> canceled = nullptr);

//  [[script::bindings::auto]]
    void addStringBundle(const String& name, const sp<StringBundle>& stringBundle);
//  [[script::bindings::auto]]
    sp<String> getString(const String& resid, const sp<String>& defValue = nullptr);
//  [[script::bindings::auto]]
    Vector<String> getStringArray(const String& resid);

//  [[script::bindings::auto]]
    sp<Runnable> defer(const sp<Runnable>& task) const;

//  [[script::bindings::property]]
    const Color& backgroundColor() const;
//  [[script::bindings::property]]
    void setBackgroundColor(const Color& backgroundColor);

private:
    sp<ApplicationContext> _context;
    sp<ApplicationController> _controller;
    sp<SurfaceController> _surface_controller;
    sp<Size> _surface_size;

    sp<Activity> _activity;
    sp<Discarded> _activity_discarded;
};

}
