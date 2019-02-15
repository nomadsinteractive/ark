#include "platform/android/impl/application/android_application.h"

#include "core/inf/runnable.h"
#include "core/util/log.h"

#include "graphics/base/surface_controller.h"

#include "app/base/application_context.h"
#include "app/base/application_controller.h"
#include "app/base/surface.h"

namespace ark {
namespace platform {
namespace android {

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

static int32_t engineHandleInput(android_app* , AInputEvent* )
{
    return 0;
}

static void engineHandleCmd(android_app* pApp, int32_t cmd)
{
    switch (cmd)
    {
    case APP_CMD_RESUME:
    {
        break;
    }

    case APP_CMD_PAUSE:
    {
        break;
    }

    case APP_CMD_INIT_WINDOW:
        if (pApp->window != nullptr)
        {
            LOGD("Initializing platform!\n");
        }
        break;

    case APP_CMD_TERM_WINDOW:
        break;
    }
}

AndroidApplication::AndroidApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport, android_app* state)
    : Application(applicationDelegate, applicationContext, width, height, viewport), _controller(sp<AndroidApplicationController>::make()), _state(state)
{
}

int AndroidApplication::run()
{
    DCHECK(_state, "Please pass android_app argument from android_main if you want to run this application");
    _state->onAppCmd = engineHandleCmd;
    _state->onInputEvent = engineHandleInput;

    while(true)
    {
        struct android_poll_source* source;
        int events;

        while(ALooper_pollAll(0, nullptr, &events, (void **)&source) >= 0)
        {
            if (source)
                source->process(_state, source);

            if (_state->destroyRequested)
                return 0;
        }

/*
        if (engine.pVulkanApp && engine.active)
        {
            unsigned index;
            vector<VkImage> images;
            Platform::SwapchainDimensions dim;

            Result res = platform.acquireNextImage(&index);
            while (res == RESULT_ERROR_OUTDATED_SWAPCHAIN)
            {
                platform.acquireNextImage(&index);
                platform.getCurrentSwapchain(&images, &dim);
                engine.pVulkanApp->updateSwapchain(images, dim);
            }

            if (FAILED(res))
            {
                LOGE("Unrecoverable swapchain error.\n");
                break;
            }

            engine.pVulkanApp->render(index, 0.0166f);
            res = platform.presentImage(index);

            // Handle Outdated error in acquire.
            if (FAILED(res) && res != RESULT_ERROR_OUTDATED_SWAPCHAIN)
                break;

            frameCount++;
            if (frameCount == 100)
            {
                double endTime = OS::getCurrentTime();
                LOGI("FPS: %.3f\n", frameCount / (endTime - startTime));
                frameCount = 0;
                startTime = endTime;
            }
        }
*/
    }
    return 0;
}

const sp<ApplicationController>& AndroidApplication::controller()
{
    return _controller;
}

}
}
}
