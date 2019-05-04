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
    Application(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport);
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
    bool onEvent(const Event& event, bool mapViewport);

    const char* name() const;
    const sp<ApplicationContext>& context() const;

    static void __parse_opt__(int32_t argc, const char* argv[]);

protected:
    sp<ApplicationDelegate> _application_delegate;
    sp<ApplicationContext> _application_context;
    sp<Surface> _surface;
    sp<SurfaceUpdater> _surface_updater;
    Viewport _viewport;
    uint32_t _width, _height;

private:
    void onCreateTask();
    void onPauseTask();
    void onResumeTask();
    void onDestroyTask();
    void onEventTask(const Event& event);

private:
    bool _alive;
};

}

#endif
