#ifndef ARK_BASE_APP_APPLICATION_H_
#define ARK_BASE_APP_APPLICATION_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Application {
public:
    Application(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t surfaceWidth, uint32_t surfaceHeight, const Viewport& viewport);
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

    static void __parse_opt__(int32_t argc, const char* argv[]);

protected:
    sp<ApplicationDelegate> _application_delegate;
    sp<ApplicationContext> _application_context;
    sp<Surface> _surface;
    Viewport _viewport;
    sp<Size> _surface_size;

private:
    void onCreateTask();
    void onPauseTask();
    void onResumeTask();
    void onDestroyTask();
    void onEventTask(const Event& event);

    void setSurfaceUpdater(bool alive);

private:
    sp<Runnable> _surface_updater_pre_created;
    sp<Runnable> _surface_updater_created;
    Runnable* _surface_updater;
};

}

#endif
