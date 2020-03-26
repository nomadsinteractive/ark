#include "platform/android/impl/application/android_application.h"

#include <android/input.h>
#include <android/keycodes.h>

#include "core/inf/runnable.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/surface_controller.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"

#include "app/base/application_context.h"
#include "app/base/application_controller.h"
#include "app/base/event.h"
#include "app/base/surface.h"

#include "impl/platform_android.h"

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

static int32_t engineHandleInput(android_app* state, AInputEvent* inputEvent)
{
    AndroidApplication* app = reinterpret_cast<AndroidApplication*>(state->userData);
    int32_t type = AInputEvent_getType(inputEvent);
    if(type == AINPUT_EVENT_TYPE_MOTION)
    {
        int32_t action = AMotionEvent_getAction(inputEvent);
        int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        action = action & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(inputEvent, pointerIndex), y = AMotionEvent_getY(inputEvent, pointerIndex);
        uint32_t timestamp = static_cast<uint32_t>(AMotionEvent_getEventTime(inputEvent) >> 10);
        Event::Action s = Event::ACTION_KEY_NONE;
        switch(action)
        {
            case AMOTION_EVENT_ACTION_DOWN:
                s = Event::ACTION_DOWN;
                break;
            case AMOTION_EVENT_ACTION_UP:
                s = Event::ACTION_UP;
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                s = Event::ACTION_MOVE;
                break;
            case AMOTION_EVENT_ACTION_CANCEL:
                s = Event::ACTION_CANCEL;
                break;
            default:
                break;
        }
        app->onEvent(Event(s, x, y, timestamp), true);
    }
    else if(type == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t keycode = AKeyEvent_getKeyCode(inputEvent);
        uint32_t timestamp = static_cast<uint32_t>(AKeyEvent_getEventTime(inputEvent) >> 10);
        if(keycode == AKEYCODE_BACK)
            app->onEvent(Event(Event::ACTION_BACK_PRESSED, 0, 0, timestamp), false);
    }
    return 0;
}

static void engineHandleCmd(android_app* state, int32_t cmd)
{
    AndroidApplication* app = reinterpret_cast<AndroidApplication*>(state->userData);
    switch (cmd)
    {
    case APP_CMD_RESUME:
        app->onResume();
        break;

    case APP_CMD_PAUSE:
        app->onPause();
        break;

    case APP_CMD_INIT_WINDOW:
        if (state->window != nullptr)
        {
            RenderEngineContext::Info& info = app->context()->renderEngine()->context()->info();
            info.android.window = state->window;
            app->context()->postToRenderer([app]() {
                app->onSurfaceCreated();
                app->context()->updateRenderState();
            });
        }
        break;

    case APP_CMD_WINDOW_RESIZED:
        app->context()->postToRenderer([app, state]() {
            app->onSurfaceChanged(state->contentRect.right - state->contentRect.left, state->contentRect.bottom - state->contentRect.top);
        });
        break;

    case APP_CMD_TERM_WINDOW:
        state->destroyRequested = true;
        break;
    default:
        break;
    }
}

AndroidApplication::AndroidApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport)
    : Application(applicationDelegate, applicationContext, width, height, viewport), _controller(sp<AndroidApplicationController>::make())
{
}

static void onContentRectChanged(ANativeActivity* activity, const ARect* rect)
{
    const Global<PlatformAndroid> platform;
    android_app* state = platform->state();
    state->contentRect = *rect;
    engineHandleCmd(state, APP_CMD_WINDOW_RESIZED);
}

int AndroidApplication::run()
{
    const Global<PlatformAndroid> platform;
    android_app* state = platform->state();

    state->userData = this;
    state->onAppCmd = engineHandleCmd;
    state->onInputEvent = engineHandleInput;
/*
 * It seems the latest android_native_app_glue.cpp still has onContentRectChanged unimplemented.
 */
    state->activity->callbacks->onContentRectChanged = onContentRectChanged;
    
    onCreate();

    while(true)
    {
        struct android_poll_source* source;
        int events;

        while(ALooper_pollAll(1, nullptr, &events, (void **)&source) >= 0)
        {
            if (source)
                source->process(state, source);

            if (state->destroyRequested)
                return 0;
        }
        onSurfaceUpdate();
    }
}

const sp<ApplicationController>& AndroidApplication::controller()
{
    return _controller;
}

}
}
}
