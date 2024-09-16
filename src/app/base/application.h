#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Application {
public:
    Application(sp<ApplicationDelegate> applicationDelegate, sp<ApplicationContext> applicationContext, uint32_t surfaceWidth, uint32_t surfaceHeight, const Viewport& viewport);
    virtual ~Application();

    virtual int run() = 0;

    virtual const sp<ApplicationController>& controller() = 0;

    virtual void onCreate();
    virtual void onPause();
    virtual void onResume();
    virtual void onDestroy();

    void onSurfaceCreated();
    void onSurfaceChanged(uint32_t width, uint32_t height);
    void onSurfaceUpdate();
    bool onEvent(const Event& event);

    V2 toViewportPosition(const V2& xy) const;

    const char* name() const;
    const sp<ApplicationContext>& context() const;
    const sp<Size>& surfaceSize() const;

protected:
    sp<ApplicationDelegate> _application_delegate;
    sp<ApplicationContext> _application_context;
    sp<Surface> _surface;
    Viewport _viewport;
    sp<Size> _surface_size;

private:
    void onCreateTask();
    void onPauseTask() const;
    void onResumeTask() const;
    void onEventTask(const Event& event) const;

    void setSurfaceUpdater(bool alive);

private:
    sp<Runnable> _surface_updater_pre_created;
    sp<Runnable> _surface_updater_created;
    Runnable* _surface_updater;
};

}
