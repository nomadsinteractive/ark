#ifndef ARK_RENDERER_BASE_RENDER_CONTEXT_H_
#define ARK_RENDERER_BASE_RENDER_CONTEXT_H_

#include <map>

#ifdef ARK_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include "core/ark.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"

#include "renderer/forwarding.h"

#ifdef ARK_PLATFORM_DARWIN
#ifdef __OBJC__
@class NSWindow;
#else
typedef struct _NSWindow NSWindow;
#endif
#endif

namespace ark {

class RenderContext {
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
                Display *display;           /**< The X11 display */
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
                ANativeWindow *window;
                EGLSurface surface;
            } android;
#endif
            uint8_t dummy[64] = {0};
    };
public:
    RenderContext(Ark::RendererVersion version, const Viewport& viewport);

    Ark::RendererVersion version() const;
    void setVersion(Ark::RendererVersion version);

    const std::map<String, String>& annotations() const;
    std::map<String, String>& annotations();

    const Viewport& viewport() const;

    void setSnippetFactory(sp<SnippetFactory> snippetfactory);
    const sp<SnippetFactory>& snippetFactory() const;

    uint32_t getGLSLVersion() const;

    const Info& info() const;
    Info& info();

private:
    Ark::RendererVersion _version;
    std::map<String, String> _annotations;

    Viewport _viewport;
    sp<SnippetFactory> _snippet_factory;

    Info _info;
};

}

#endif
