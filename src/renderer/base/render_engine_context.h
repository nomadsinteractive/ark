#ifndef ARK_RENDERER_BASE_RENDER_ENGINE_CONTEXT_H_
#define ARK_RENDERER_BASE_RENDER_ENGINE_CONTEXT_H_

#include <map>

#ifdef ARK_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef ARK_PLATFORM_ANDROID
#include <android/native_window.h>
#include <EGL/egl.h>
#endif

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

#ifdef ARK_PLATFORM_DARWIN
#ifdef __OBJC__
@class NSWindow;
#else
typedef struct _NSWindow NSWindow;
#endif
#endif

namespace ark {

class ARK_API RenderEngineContext {
public:
    union Info {
#ifdef ARK_PLATFORM_WINDOWS
            struct {
                HWND window;                /**< The window handle */
                HDC hdc;                    /**< The window device context */
                HINSTANCE hinstance;        /**< The instance handle */
            } windows;
#endif
#ifdef ARK_PLATFORM_LINUX
            struct {
                Display* display;           /**< The X11 display */
                Window window;              /**< The X11 window */
            } x11;
#endif
#ifdef ARK_PLATFORM_DARWIN
            struct {
                NSWindow* window;
                void* view;
            } darwin;
#endif
#ifdef ARK_PLATFORM_ANDROID
            struct {
                ANativeWindow* window;
                EGLSurface surface;
            } android;
#endif
            uint8_t dummy[64] = {0};
    };

    struct Resolution {
        uint32_t width;
        uint32_t height;
    };

public:
    RenderEngineContext(Ark::RendererVersion version, Ark::RendererCoordinateSystem coordinateSystem, const Viewport& viewport);

    Ark::RendererVersion version() const;
    void setVersion(Ark::RendererVersion version);

    Ark::RendererCoordinateSystem coordinateSystem() const;

    const std::map<String, String>& definitions() const;
    std::map<String, String>& definitions();

    const Viewport& viewport() const;
    void setViewport(const Viewport& viewport);

    const Resolution& displayResolution() const;
    void setDisplayResolution(const Resolution& displayResolution);

    const V2& displayUnit() const;

    void setSnippetFactory(sp<SnippetFactory> snippetfactory);
    const sp<SnippetFactory>& snippetFactory() const;

    uint32_t getGLSLVersion() const;

    const Info& info() const;
    Info& info();

private:
    Ark::RendererVersion _version;
    Ark::RendererCoordinateSystem _coordinate_system;
    Viewport _viewport;

    Info _info;

    std::map<String, String> _annotations;
    sp<SnippetFactory> _snippet_factory;
    Resolution _display_resolution;
    V2 _display_unit;
};

}

#endif
