#include "platform/android/impl/application/android_application.h"

#include "core/inf/runnable.h"

#include "graphics/base/surface_controller.h"

#include "app/base/application_context.h"
#include "app/base/application_controller.h"
#include "app/base/surface.h"

#include "util/jni_util.h"

namespace ark {
namespace platform {
namespace android {

RenderRequest gRenderRequest;

namespace {

class AndroidApplicationController : public ApplicationController {
public:
    AndroidApplicationController() {
    }

    virtual sp<Object> createCursor(const sp<Bitmap>& bitmap, uint32_t hotX, uint32_t hotY) override {
        return nullptr;
    }

    virtual sp<Object> createSystemCursor(ApplicationController::SystemCursorName name) override {
        return nullptr;
    }

    virtual void showCursor(const sp<Object>& cursor) override {
    }

    virtual void hideCursor() override {
    }

    virtual void exit() override {
    }
};

}

AndroidApplication::AndroidApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport)
    : Application(applicationDelegate, applicationContext, width, height, viewport), _controller(sp<AndroidApplicationController>::make())
{
}

int AndroidApplication::run()
{
    return 0;
}

const sp<ApplicationController>& AndroidApplication::controller()
{
    return _controller;
}

void AndroidApplication::onCreate()
{
    Application::onCreate();

    _surface->scheduleUpdate(_application_context, 60);
}

}
}
}
