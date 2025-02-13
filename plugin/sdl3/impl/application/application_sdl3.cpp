#include "sdl3/impl/application/application_sdl3.h"

#ifdef ARK_USE_OPEN_GL
#include <SDL3/SDL_opengl.h>
#endif

#ifdef ARK_USE_VULKAN
#include <SDL3/SDL_vulkan.h>
#endif

#include "core/base/clock.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"
#include "app/base/event.h"
#include "app/impl/application/application_delegate_impl.h"
#include "app/inf/application_controller.h"

#include "renderer/inf/renderer_factory.h"

#include "sdl3/base/context_sdl3_gpu.h"

#ifdef ARK_PLATFORM_DARWIN
struct SDL_VideoDevice;

extern "C" void* Cocoa_Metal_CreateView(SDL_VideoDevice* _this, SDL_Window* window);
#endif

namespace ark::plugin::sdl3 {

namespace {

bool gQuit = false;

Event::Code sdlScanCodeToEventCode(SDL_Scancode sc)
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

void doSetMouseCapture(bool enabled)
{
    const int32_t r = SDL_CaptureMouse(enabled);
    CHECK_WARN(r == 0, "Error calling SDL_CaptureMouse, enabled: %d, return: %d, error: %s", enabled, r, SDL_GetError());
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void* pixels, int32_t width, int32_t height, int32_t depth, int32_t pitch, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
    return SDL_CreateSurfaceFrom(width, height, SDL_GetPixelFormatForMasks(depth, rmask, gmask, bmask, amask), pixels, pitch);
}

class SDLCursor {
public:
    SDLCursor(SDL_Cursor* cursor)
        : _cursor(cursor) {
    }

    ~SDLCursor() {
        if(_cursor)
            SDL_DestroyCursor(_cursor);
    }

    SDL_Cursor* cursor() const {
        return _cursor;
    }

private:
    SDL_Cursor* _cursor;
};

class SDLApplicationController final : public ApplicationController {
public:
    SDLApplicationController(sp<ApplicationContext> applicationContext)
        : _application_context(std::move(applicationContext)) {
    }

    Box createCursor(const sp<Bitmap>& bitmap, int32_t hotX, int32_t hotY) override {
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
        SDL_DestroySurface(surface);
        return Box(sp<SDLCursor>::make(cursor));
    }

    Box createSystemCursor(SystemCursorName name) override {
        SDL_Cursor* cursor = nullptr;
        switch(name) {
            case SYSTEM_CURSOR_DEFAULT:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
            break;
            case SYSTEM_CURSOR_TEXT:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
            break;
            case SYSTEM_CURSOR_WAIT:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
            break;
            case SYSTEM_CURSOR_CROSSHAIR:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
            break;
            case SYSTEM_CURSOR_PROGRESS:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
            break;
            case SYSTEM_CURSOR_SIZENWSE:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
            break;
            case SYSTEM_CURSOR_SIZENESW:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
            break;
            case SYSTEM_CURSOR_SIZEWE:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
            break;
            case SYSTEM_CURSOR_SIZENS:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
            break;
            case SYSTEM_CURSOR_SIZEALL:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
            break;
            case SYSTEM_CURSOR_NO:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
            break;
            case SYSTEM_CURSOR_HAND:
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
            break;
        }
        return cursor ? Box(sp<SDLCursor>::make(cursor)) : nullptr;
    }

    void showCursor(const Box& cursor) override {
        if(!SDL_CursorVisible())
            SDL_ShowCursor();

        if(cursor) {
            const sp<SDLCursor> s = cursor.as<SDLCursor>();
            CHECK(s, "Object is not a SDLCursor instance");
            if(s)
                SDL_SetCursor(s->cursor());
        }
    }

    void hideCursor() override {
        SDL_HideCursor();
    }

    void setMouseCapture(bool enabled) override {
#ifdef _WIN32
        _application_context->messageLoopRenderer()->post([enabled] () {
            doSetMouseCapture(enabled);
        }, 0);
#else
        doSetMouseCapture(enabled);
#endif
    }

    void exit() override {
        gQuit = true;
    }

private:
    sp<ApplicationContext> _application_context;
};

V2 toFragCoordXY(const V2& xy, Ark::RendererCoordinateSystem rcs, float surfaceHeight)
{
    if(rcs == Ark::COORDINATE_SYSTEM_RHS)
        return {xy.x(), surfaceHeight - xy.y()};
    return xy;
}

int32_t toWindowPosition(int32_t pos)
{
    if(pos == ApplicationManifest::WINDOW_POSITION_UNDEFINED)
        return SDL_WINDOWPOS_UNDEFINED;
    if(pos == ApplicationManifest::WINDOW_POSITION_CENTERED)
        return SDL_WINDOWPOS_CENTERED;
    return pos;
}

}

ApplicationSDL3::ApplicationSDL3(sp<ApplicationDelegate> applicationDelegate, sp<ApplicationContext> applicationContext, const ApplicationManifest& manifest)
    : Application(std::move(applicationDelegate), applicationContext, manifest), _main_window(nullptr), _cond(SDL_CreateCondition()), _lock(SDL_CreateMutex()),
      _controller(sp<SDLApplicationController>::make(std::move(applicationContext))), _vsync(manifest.renderer()._vsync)
{
    initialize();
}

int ApplicationSDL3::run()
{
    /* Create our opengl context and attach it to our window */
    const SDL_GLContext maincontext = _use_open_gl ? SDL_GL_CreateContext(_main_window) : nullptr;

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    if(_use_open_gl)
        SDL_GL_SetSwapInterval(_vsync ? 1 : 0);

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

        if(_vsync)
            SDL_Delay(1);
    }

    onDestroy();

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    if(_use_open_gl)
        SDL_GL_DestroyContext(maincontext);

    SDL_DestroyWindow(_main_window);

    SDL_DestroyCondition(_cond);
    SDL_DestroyMutex(_lock);

    SDL_Quit();

    return 0;
}

const sp<ApplicationController>& ApplicationSDL3::controller()
{
    return _controller;
}

void ApplicationSDL3::onSurfaceChanged()
{
    int32_t w, h;
    SDL_GetWindowSize(_main_window, &w, &h);
    Application::onSurfaceChanged(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

sp<Application> ApplicationSDL3::BUILDER::build(const Scope& args)
{
    const Ark& ark = Ark::instance();
    const sp<ApplicationManifest>& manifest = ark.manifest();
    return sp<Application>::make<ApplicationSDL3>(sp<ApplicationDelegate>::make<ApplicationDelegateImpl>(), ark.applicationContext(), manifest);
}

void ApplicationSDL3::initialize()
{
    /* Initialize SDL's Video subsystem */
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        FATAL("Unable to initialize SDL");
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /* Turn on double buffering with a 24bit Z buffer.
    * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    const ApplicationManifest::Window& window = Ark::instance().manifest()->window();

    if(window._flags.has(ApplicationManifest::WINDOW_FLAG_SHOW_CURSOR))
        SDL_ShowCursor();
    else
        SDL_HideCursor();

    RenderEngine& renderEngine = _application_context->renderEngine();
    _use_open_gl = renderEngine.version() < Ark::RENDERER_VERSION_VULKAN_11;

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, window._title.c_str());
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, toWindowPosition(window._position_x));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, toWindowPosition(window._position_y));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, static_cast<int32_t>(_surface_size->widthAsFloat()));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, static_cast<int32_t>(_surface_size->heightAsFloat()));

    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, window._flags.has(ApplicationManifest::WINDOW_FLAG_FULL_SCREEN));
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, window._flags.has(ApplicationManifest::WINDOW_FLAG_FULL_SCREEN_WINDOWED));
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_MAXIMIZED_BOOLEAN, window._flags.has(ApplicationManifest::WINDOW_FLAG_MAXINIZED));
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, window._flags.has(ApplicationManifest::WINDOW_FLAG_RESIZABLE));

    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, _use_open_gl);

    _main_window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if(!_main_window)
    {
        /* Die if creation failed */
        SDL_Quit();
        FATAL(SDL_GetError());
    }

    ContextSDL3_GPU& sdl3GPUContext = renderEngine.context()->traits().ensure<ContextSDL3_GPU>();
    sdl3GPUContext._main_window = _main_window;

    RenderEngine::PlatformInfo& info = renderEngine.info();
#if defined(ARK_PLATFORM_WINDOWS)
    info.windows.hinstance = static_cast<HINSTANCE>(SDL_GetPointerProperty(SDL_GetWindowProperties(_main_window), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
    info.windows.hdc = static_cast<HDC>(SDL_GetPointerProperty(SDL_GetWindowProperties(_main_window), SDL_PROP_WINDOW_WIN32_HDC_POINTER, nullptr));
    info.windows.window = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(_main_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
#elif defined (ARK_PLATFORM_DARWIN)
    info.darwin.window = static_cast<NSWindow*>(SDL_GetPointerProperty(SDL_GetWindowProperties(_main_window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr));
    info.darwin.view = Cocoa_Metal_CreateView(nullptr, _main_window);
#endif
}

void ApplicationSDL3::pollEvents(uint64_t timestamp)
{
    const Ark::RendererCoordinateSystem rcs = _application_context->renderController()->renderEngine()->rendererFactory()->features()._default_coordinate_system;
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_EVENT_QUIT:
            gQuit = true;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                const V2 rawPosition(static_cast<float>(event.button.x), static_cast<float>(event.button.y));
                const Event::Button which = static_cast<Event::Button>(Event::BUTTON_MOUSE_LEFT + event.button.button - SDL_BUTTON_LEFT);
                Event::ButtonInfo bi{toViewportPosition(rawPosition), toFragCoordXY(rawPosition, rcs, _surface_size->heightAsFloat()), which};
                Event e(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? Event::ACTION_DOWN : Event::ACTION_UP, timestamp, bi);
                onEvent(e);
                break;
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                const V2 rawPosition(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
                const Event::Button which = static_cast<Event::Button>(Event::BUTTON_MOUSE_LEFT + event.button.button - SDL_BUTTON_LEFT);
                Event::MotionInfo mi{toViewportPosition(rawPosition), toFragCoordXY(rawPosition, rcs, _surface_size->heightAsFloat()), which, event.motion.state};
                Event e(Event::ACTION_MOVE, timestamp, mi);
                onEvent(e);
                break;
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                const V2 rawPosition(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
                const Event::Button which = static_cast<Event::Button>(Event::BUTTON_MOUSE_LEFT + event.button.button - SDL_BUTTON_LEFT);
                Event::MotionInfo mi{rawPosition, rawPosition, which, event.motion.state};
                Event e(Event::ACTION_WHEEL, timestamp, mi);
                onEvent(e);
                break;
            }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            {
                const Event::KeyboardInfo keyboardInfo{sdlScanCodeToEventCode(event.key.scancode), static_cast<wchar_t>(event.key.key)};
                const Event e(event.key.repeat ? Event::ACTION_KEY_REPEAT : (event.type == SDL_EVENT_KEY_DOWN ? Event::ACTION_KEY_DOWN : Event::ACTION_KEY_UP), timestamp, keyboardInfo);
                onEvent(e);
                break;
            }
        case SDL_EVENT_TEXT_INPUT:
            {
                const Event e(Event::ACTION_TEXT_INPUT, timestamp, Event::TextInputInfo(event.text.text));
                onEvent(e);
                break;
            }
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_MINIMIZED:
            onPause();
            break;
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_RESTORED:
            onResume();
            onSurfaceChanged();
            break;
        }
    }
}

}
