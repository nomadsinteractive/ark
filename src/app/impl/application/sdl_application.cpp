#include "app/impl/application/sdl_application.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>

#ifdef ARK_USE_VULKAN
#include <SDL_vulkan.h>
#endif

#include "core/base/clock.h"
#include "core/base/object.h"
#include "core/impl/message_loop/message_loop_default.h"
#include "core/inf/runnable.h"
#include "core/types/class.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/surface_controller.h"
#include "graphics/inf/render_view.h"

#include "app/base/application_context.h"
#include "app/base/application_controller.h"
#include "app/base/surface.h"

#include "platform/platform.h"


namespace ark {

static bool gQuit = false;

namespace {

class SDLCursor : public Object, public Implements<SDLCursor, Object> {
public:
    SDLCursor(SDL_Cursor* cursor)
        : _cursor(cursor) {
    }

    ~SDLCursor() {
        if(_cursor)
            SDL_FreeCursor(_cursor);
    }

    SDL_Cursor* cursor() {
        return _cursor;
    }

private:
    SDL_Cursor* _cursor;
};

class SDLApplicationController : public ApplicationController {
public:
    virtual sp<Object> createCursor(const sp<Bitmap>& bitmap, uint32_t hotX, uint32_t hotY) override {
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(bitmap->at(0, 0), bitmap->width(), bitmap->height(),
                                                        32, bitmap->rowBytes(), 0x00FF0000, 0x0000FF00,
                                                        0x000000FF, 0xFF000000);
        SDL_Cursor* cursor = SDL_CreateColorCursor(surface, hotX, hotY);
        SDL_FreeSurface(surface);
        return sp<SDLCursor>::make(cursor);
    }

    virtual sp<Object> createSystemCursor(ApplicationController::SystemCursorName name) override {
        SDL_Cursor* cursor = nullptr;
        switch(name) {
            case ApplicationController::SYSTEM_CURSOR_ARROW:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            break;
            case ApplicationController::SYSTEM_CURSOR_IBEAM:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
            break;
            case ApplicationController::SYSTEM_CURSOR_WAIT:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
            break;
            case ApplicationController::SYSTEM_CURSOR_CROSSHAIR:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
            break;
            case ApplicationController::SYSTEM_CURSOR_WAITARROW:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
            break;
            case ApplicationController::SYSTEM_CURSOR_SIZENWSE:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
            break;
            case ApplicationController::SYSTEM_CURSOR_SIZENESW:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
            break;
            case ApplicationController::SYSTEM_CURSOR_SIZEWE:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
            break;
            case ApplicationController::SYSTEM_CURSOR_SIZENS:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
            break;
            case ApplicationController::SYSTEM_CURSOR_SIZEALL:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
            break;
            case ApplicationController::SYSTEM_CURSOR_NO:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
            break;
            case ApplicationController::SYSTEM_CURSOR_HAND:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
            break;
        }
        return cursor ? sp<SDLCursor>::make(cursor) : nullptr;
    }

    virtual void showCursor(const sp<Object>& cursor) override {
        if(SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE)
            SDL_ShowCursor(SDL_ENABLE);

        const sp<SDLCursor> s = cursor.as<SDLCursor>();
        DCHECK(s, "Object is not a SDLCursor instance");
        if(s)
            SDL_SetCursor(s->cursor());
    }

    virtual void hideCursor() override {
        SDL_ShowCursor(SDL_DISABLE);
    }

    virtual void exit() override {
        gQuit = true;
    }
};

class SDLPollEventTask : public Runnable {
public:
    SDLPollEventTask(SDLApplication& application)
        : _application(application) {
    }

    virtual void run() override {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                    gQuit = true;
                    break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                {
                    Event::Code code = static_cast<Event::Code>(Event::CODE_MOUSE_BUTTON_LEFT + event.button.button - SDL_BUTTON_LEFT);
                    Event e(event.type == SDL_MOUSEBUTTONDOWN ? Event::ACTION_DOWN : Event::ACTION_UP, event.button.x, event.button.y, event.button.timestamp, code);
                    _application.onEvent(e, true);
                    break;
                }
            case SDL_MOUSEMOTION:
                {
                    Event e(Event::ACTION_MOVE, event.motion.x, event.motion.y, event.motion.timestamp);
                    _application.onEvent(e, true);
                    break;
                }
            case SDL_MOUSEWHEEL:
                {
                    Event e(Event::ACTION_WHEEL, event.wheel.x, event.wheel.y, event.wheel.timestamp);
                    _application.onEvent(e, false);
                    break;
                }
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                {
                    Event e(event.key.repeat ? Event::ACTION_KEY_REPEAT : (event.type == SDL_KEYDOWN ? Event::ACTION_KEY_DOWN : Event::ACTION_KEY_UP), event.key.timestamp, sdlScanCodeToEventCode(event.key.keysym.scancode));
                    _application.onEvent(e, false);
                    break;
                }
            case SDL_WINDOWEVENT:
                {
                    switch (event.window.event)  {
                    case SDL_WINDOWEVENT_HIDDEN:
                    case SDL_WINDOWEVENT_MINIMIZED:
                        _application.onPause();
                        break;
                    case SDL_WINDOWEVENT_SHOWN:
                    case SDL_WINDOWEVENT_RESTORED:
                        _application.onResume();
                        _application.onSurfaceChanged();
                    }
                    break;
                }
            }
        }
    }

    Event::Code sdlScanCodeToEventCode(SDL_Scancode sc) {
        if(Math::between<SDL_Scancode>(SDL_SCANCODE_A, SDL_SCANCODE_Z, sc))
            return static_cast<Event::Code>(Event::CODE_KEYBOARD_A + sc - SDL_SCANCODE_A);
        if(Math::between<SDL_Scancode>(SDL_SCANCODE_F1, SDL_SCANCODE_F12, sc))
            return static_cast<Event::Code>(Event::CODE_KEYBOARD_F1 + sc - SDL_SCANCODE_F1);
        switch(sc) {
        case SDL_SCANCODE_GRAVE:
            return Event::CODE_KEYBOARD_GRAVE;
        case SDL_SCANCODE_RIGHT:
            return Event::CODE_KEYBOARD_RIGHT;
        case SDL_SCANCODE_LEFT:
            return Event::CODE_KEYBOARD_LEFT;
        case SDL_SCANCODE_DOWN:
            return Event::CODE_KEYBOARD_DOWN;
        case SDL_SCANCODE_UP:
            return Event::CODE_KEYBOARD_UP;
        case SDL_SCANCODE_RETURN:
            return Event::CODE_KEYBOARD_RETURN;
        case SDL_SCANCODE_ESCAPE:
            return Event::CODE_KEYBOARD_ESCAPE;
        case SDL_SCANCODE_BACKSPACE:
            return Event::CODE_KEYBOARD_BACKSPACE;
        case SDL_SCANCODE_TAB:
            return Event::CODE_KEYBOARD_TAB;
        case SDL_SCANCODE_SPACE:
            return Event::CODE_KEYBOARD_SPACE;
        default:
            break;
        }
        return Event::CODE_NONE;
    }

private:
    SDLApplication& _application;

};

}

#ifdef _WIN32
    HINSTANCE gInstance;
    HWND gWnd;
#endif

SDLApplication::SDLApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport, uint32_t windowFlag)
    : Application(applicationDelegate, applicationContext, width, height, viewport), _main_window(nullptr), _cond(SDL_CreateCond()), _lock(SDL_CreateMutex())
      , _message_loop_rendering(sp<MessageLoopDefault>::make(Platform::getSteadyClock())), _controller(sp<SDLApplicationController>::make())
      , _show_cursor(windowFlag & WINDOW_FLAG_SHOW_CURSOR), _window_flag(SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)
{
    if(windowFlag & WINDOW_FLAG_FULL_SCREEN)
        _window_flag |= SDL_WINDOW_FULLSCREEN;
    if(windowFlag & WINDOW_FLAG_FULL_SCREEN_WINDOWED)
        _window_flag |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    if(windowFlag & WINDOW_FLAG_MAXINIZED)
        _window_flag |= SDL_WINDOW_MAXIMIZED;
    if(windowFlag & WINDOW_FLAG_RESIZABLE)
        _window_flag |= SDL_WINDOW_RESIZABLE;
}

int SDLApplication::run()
{
    /* Initialize SDL's Video subsystem */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        DFATAL("Unable to initialize SDL");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /* Turn on double buffering with a 24bit Z buffer.
    * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_ShowCursor(_show_cursor ? SDL_ENABLE : SDL_DISABLE);

    /* Create our window centered at 512x512 resolution */
    _main_window = SDL_CreateWindow(name(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width, _height, _window_flag);
    if(!_main_window)
    {
        /* Die if creation failed */
        SDL_Quit();
        DFATAL(SDL_GetError());
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    //We will get to that later
    auto result = SDL_GetWindowWMInfo(_main_window, &wmInfo);
    DASSERT(result);
#ifdef _WIN32
    gInstance = wmInfo.info.win.hinstance;
    gWnd = wmInfo.info.win.window;
#endif


#ifndef ARK_USE_VULKAN
    /* Create our opengl context and attach it to our window */
    SDL_GLContext maincontext = SDL_GL_CreateContext(_main_window);

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);
#endif

    onCreate();
    onSurfaceCreated();

    _message_loop_rendering->schedule(sp<SDLPollEventTask>::make(*this), 0.02f);
    _surface->scheduleUpdate(_application_context, 60);

    while(!gQuit)
    {
        _message_loop_rendering->pollOnce();
        onSurfaceDraw();
#ifndef ARK_USE_VULKAN
        SDL_GL_SwapWindow(_main_window);
#endif
        SDL_Delay(1);
    }

    onDestroy();

    /* Delete our opengl context, destroy our window, and shutdown SDL */
#ifndef ARK_USE_VULKAN
    SDL_GL_DeleteContext(maincontext);
#endif
    SDL_DestroyWindow(_main_window);

    SDL_DestroyCond(_cond);
    SDL_DestroyMutex(_lock);

    SDL_Quit();

    return 0;
}

const sp<ApplicationController>& SDLApplication::controller()
{
    return _controller;
}

void SDLApplication::onSurfaceChanged()
{
    int32_t w, h;
    SDL_GetWindowSize(_main_window, &w, &h);
    Application::onSurfaceChanged(_width = w, _height = h);
}

}
