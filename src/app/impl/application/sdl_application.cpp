#include "app/impl/application/sdl_application.h"

#include <SDL.h>

#ifdef ARK_USE_OPEN_GL
#include <SDL_opengl.h>
#endif

#ifdef ARK_USE_VULKAN
#include <SDL_vulkan.h>
#endif

#include <SDL_syswm.h>

#include "core/base/clock.h"
#include "core/base/message_loop.h"
#include "core/base/object.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"
#include "graphics/base/surface_controller.h"
#include "graphics/inf/render_view.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"
#include "app/base/surface.h"
#include "app/base/surface_updater.h"
#include "app/inf/application_controller.h"

#include "platform/platform.h"

#ifdef ARK_PLATFORM_DARWIN
struct SDL_VideoDevice;

extern "C" void* Cocoa_Metal_CreateView(SDL_VideoDevice* _this, SDL_Window* window);
#endif

namespace ark {

static bool gQuit = false;

static Event::Code sdlScanCodeToEventCode(SDL_Scancode sc)
{
    if(Math::between<SDL_Scancode>(SDL_SCANCODE_A, SDL_SCANCODE_Z, sc))
        return static_cast<Event::Code>(static_cast<int32_t>(Event::CODE_KEYBOARD_A) + sc - SDL_SCANCODE_A);
    if(Math::between<SDL_Scancode>(SDL_SCANCODE_F1, SDL_SCANCODE_F12, sc))
        return static_cast<Event::Code>(static_cast<int32_t>(Event::CODE_KEYBOARD_F1) + sc - SDL_SCANCODE_F1);
    if(Math::between<SDL_Scancode>(SDL_SCANCODE_1, SDL_SCANCODE_9, sc))
        return static_cast<Event::Code>(static_cast<int32_t>(Event::CODE_KEYBOARD_1) + sc - SDL_SCANCODE_1);
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
        return Event::CODE_KEYBOARD_ENTER;
    case SDL_SCANCODE_ESCAPE:
        return Event::CODE_KEYBOARD_ESCAPE;
    case SDL_SCANCODE_BACKSPACE:
        return Event::CODE_KEYBOARD_BACKSPACE;
    case SDL_SCANCODE_DELETE:
        return Event::CODE_KEYBOARD_DELETE;
    case SDL_SCANCODE_TAB:
        return Event::CODE_KEYBOARD_TAB;
    case SDL_SCANCODE_SPACE:
        return Event::CODE_KEYBOARD_SPACE;
    case SDL_SCANCODE_0:
        return Event::CODE_KEYBOARD_0;
    case SDL_SCANCODE_LSHIFT:
        return Event::CODE_KEYBOARD_LSHIFT;
    case SDL_SCANCODE_RSHIFT:
        return Event::CODE_KEYBOARD_RSHIFT;
    case SDL_SCANCODE_LCTRL:
        return Event::CODE_KEYBOARD_LCTRL;
    case SDL_SCANCODE_RCTRL:
        return Event::CODE_KEYBOARD_RCTRL;
    case SDL_SCANCODE_LALT:
        return Event::CODE_KEYBOARD_LALT;
    case SDL_SCANCODE_RALT:
        return Event::CODE_KEYBOARD_RALT;
    case SDL_SCANCODE_COMMA:
        return Event::CODE_KEYBOARD_COMMA;
    case SDL_SCANCODE_PERIOD:
        return Event::CODE_KEYBOARD_PERIOD;
    case SDL_SCANCODE_SLASH:
        return Event::CODE_KEYBOARD_SLASH;
    case SDL_SCANCODE_SEMICOLON:
        return Event::CODE_KEYBOARD_SEMICOLON;
    case SDL_SCANCODE_APOSTROPHE:
        return Event::CODE_KEYBOARD_APOSTROPHE;
    case SDL_SCANCODE_MINUS:
        return Event::CODE_KEYBOARD_MINUS;
    case SDL_SCANCODE_EQUALS:
        return Event::CODE_KEYBOARD_EQUALS;
    case SDL_SCANCODE_LEFTBRACKET:
        return Event::CODE_KEYBOARD_LEFTBRACKET;
    case SDL_SCANCODE_RIGHTBRACKET:
        return Event::CODE_KEYBOARD_RIGHTBRACKET;
    case SDL_SCANCODE_BACKSLASH:
        return Event::CODE_KEYBOARD_BACKSLASH;
    case SDL_SCANCODE_HOME:
        return Event::CODE_KEYBOARD_HOME;
    case SDL_SCANCODE_END:
        return Event::CODE_KEYBOARD_END;
    case SDL_SCANCODE_PAGEUP:
        return Event::CODE_KEYBOARD_PAGE_UP;
    case SDL_SCANCODE_PAGEDOWN:
        return Event::CODE_KEYBOARD_PAGE_DOWN;
    default:
        break;
    }
    return Event::CODE_NONE;
}

namespace {

class SDLCursor : public Object, public Implements<SDLCursor, Object> {
public:
    SDLCursor(SDL_Cursor* cursor)
        : _cursor(cursor) {
    }

    ~SDLCursor() override {
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
    virtual sp<Object> createCursor(const sp<Bitmap>& bitmap, int32_t hotX, int32_t hotY) override {
        Uint32 rmask, gmask, bmask, amask;
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            rmask = 0xff000000;
            gmask = 0x00ff0000;
            bmask = 0x0000ff00;
            amask = 0x000000ff;
        #else // little endian, like x86
            rmask = 0x000000ff;
            gmask = 0x0000ff00;
            bmask = 0x00ff0000;
            amask = 0xff000000;
        #endif
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(bitmap->at(0, 0), static_cast<int32_t>(bitmap->width()), static_cast<int32_t>(bitmap->height()),
                                                        32, static_cast<int32_t>(bitmap->rowBytes()), rmask, gmask, bmask, amask);
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

        if(cursor) {
            const sp<SDLCursor> s = cursor.as<SDLCursor>();
            DCHECK(s, "Object is not a SDLCursor instance");
            if(s)
                SDL_SetCursor(s->cursor());
        }
    }

    virtual void hideCursor() override {
        SDL_ShowCursor(SDL_DISABLE);
    }

    virtual void setMouseCapture(bool enabled) override {
        int32_t r = SDL_CaptureMouse(enabled ? SDL_TRUE : SDL_FALSE);
        DCHECK_WARN(r == 0, "Error calling SDL_CaptureMouse, enabled: %d, return: %d", enabled, r);
    }

    virtual void exit() override {
        gQuit = true;
    }

};

}

SDLApplication::SDLApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const ApplicationManifest& manifest)
    : Application(applicationDelegate, applicationContext, width, height, manifest.renderer().toViewport()), _main_window(nullptr), _cond(SDL_CreateCond()), _lock(SDL_CreateMutex()),
      _controller(sp<SDLApplicationController>::make()), _window_flag(manifest.application()._window_flag)
{
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /* Turn on double buffering with a 24bit Z buffer.
    * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_ShowCursor((_window_flag & ApplicationManifest::WINDOW_FLAG_SHOW_CURSOR) ? SDL_ENABLE : SDL_DISABLE);

    _main_window = SDL_CreateWindow(name(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int32_t>(_surface_size->widthAsFloat()), static_cast<int32_t>(_surface_size->heightAsFloat()), toSDLWindowFlag(_application_context, _window_flag));
    if(!_main_window)
    {
        /* Die if creation failed */
        SDL_Quit();
        FATAL(SDL_GetError());
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version)

    auto result = SDL_GetWindowWMInfo(_main_window, &wmInfo);
    DASSERT(result);

    RenderEngineContext::Info& info = _application_context->renderEngine()->context()->info();

#if defined(ARK_PLATFORM_WINDOWS)
    info.windows.hinstance = wmInfo.info.win.hinstance;
    info.windows.hdc = wmInfo.info.win.hdc;
    info.windows.window = wmInfo.info.win.window;
#elif defined (ARK_PLATFORM_DARWIN)
    info.darwin.window = wmInfo.info.cocoa.window;
    info.darwin.view = Cocoa_Metal_CreateView(nullptr, _main_window);
#endif

    /* Create our opengl context and attach it to our window */
    SDL_GLContext maincontext = _use_open_gl ? SDL_GL_CreateContext(_main_window) : nullptr;

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    if(_use_open_gl)
        SDL_GL_SetSwapInterval(1);

    onCreate();
    onSurfaceCreated();
    _application_context->updateRenderState();

    while(!gQuit)
    {
        if(_application_context->isPaused())
        {
            SDL_Delay(50);
            pollEvents(_application_context->sysClock()->val());
            continue;
        }
        pollEvents(_application_context->sysClock()->val());
        onSurfaceUpdate();
        if(_use_open_gl)
            SDL_GL_SwapWindow(_main_window);
        SDL_Delay(1);
    }

    onDestroy();

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    if(_use_open_gl)
        SDL_GL_DeleteContext(maincontext);

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
    Application::onSurfaceChanged(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

uint32_t SDLApplication::toSDLWindowFlag(const sp<ApplicationContext>& applicationContext, uint32_t appWindowFlag)
{
    Ark::RendererVersion version = applicationContext->renderEngine()->version();

    uint32_t windowFlag = SDL_WINDOW_SHOWN;
    if(appWindowFlag & ApplicationManifest::WINDOW_FLAG_FULL_SCREEN)
        windowFlag |= SDL_WINDOW_FULLSCREEN;
    if(appWindowFlag & ApplicationManifest::WINDOW_FLAG_FULL_SCREEN_WINDOWED)
        windowFlag |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    if(appWindowFlag & ApplicationManifest::WINDOW_FLAG_MAXINIZED)
        windowFlag |= SDL_WINDOW_MAXIMIZED;
    if(appWindowFlag & ApplicationManifest::WINDOW_FLAG_RESIZABLE)
        windowFlag |= SDL_WINDOW_RESIZABLE;

    _use_open_gl = version < Ark::RENDERER_VERSION_VULKAN_11;
    windowFlag |= (_use_open_gl ? SDL_WINDOW_OPENGL : 0);
    return windowFlag;
}

void SDLApplication::pollEvents(uint64_t timestamp)
{
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
                Event::Button which = static_cast<Event::Button>(Event::BUTTON_MOUSE_LEFT + event.button.button - SDL_BUTTON_LEFT);
                Event::ButtonInfo bi(toViewportPosition(V2(static_cast<float>(event.button.x), static_cast<float>(event.button.y))), which);
                Event e(event.type == SDL_MOUSEBUTTONDOWN ? Event::ACTION_DOWN : Event::ACTION_UP, timestamp, bi);
                onEvent(e);
                break;
            }
        case SDL_MOUSEMOTION:
            {
                Event::Button which = static_cast<Event::Button>(Event::BUTTON_MOUSE_LEFT + event.button.button - SDL_BUTTON_LEFT);
                Event::MotionInfo mi(toViewportPosition(V2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y))), which, event.motion.state);
                Event e(Event::ACTION_MOVE, timestamp, mi);
                onEvent(e);
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                Event::Button which = static_cast<Event::Button>(Event::BUTTON_MOUSE_LEFT + event.button.button - SDL_BUTTON_LEFT);
                Event::MotionInfo mi(V2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)), which, event.motion.state);
                Event e(Event::ACTION_WHEEL, timestamp, mi);
                onEvent(e);
                break;
            }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            {
                Event::KeyboardInfo keyboardInfo(sdlScanCodeToEventCode(event.key.keysym.scancode), static_cast<wchar_t>(event.key.keysym.sym));
                Event e(event.key.repeat ? Event::ACTION_KEY_REPEAT : (event.type == SDL_KEYDOWN ? Event::ACTION_KEY_DOWN : Event::ACTION_KEY_UP), timestamp, keyboardInfo);
                onEvent(e);
                break;
            }
        case SDL_TEXTINPUT:
            {
                Event e(Event::ACTION_TEXT_INPUT, timestamp, Event::TextInputInfo(event.text.text));
                onEvent(e);
                break;
            }
        case SDL_WINDOWEVENT:
            {
                switch (event.window.event)  {
                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOWEVENT_MINIMIZED:
                    onPause();
                    break;
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_RESTORED:
                    onResume();
                    onSurfaceChanged();
                }
                break;
            }
        }
    }
}

}
