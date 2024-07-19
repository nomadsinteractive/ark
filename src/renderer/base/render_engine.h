#pragma once

#ifdef ARK_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef ARK_PLATFORM_ANDROID
#include <android/native_window.h>
#include <EGL/egl.h>
#endif

#ifdef ARK_PLATFORM_DARWIN
#ifdef __OBJC__
@class NSWindow;
#else
typedef struct _NSWindow NSWindow;
#endif
#endif

#include "core/ark.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderEngine {
public:
    union PlatformInfo {
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

public:
    RenderEngine(Ark::RendererVersion version, Ark::RendererCoordinateSystem coordinateSystem, sp<RendererFactory> rendererFactory);

    Ark::RendererVersion version() const;
    Ark::RendererCoordinateSystem coordinateSystem() const;

    const sp<RendererFactory>& rendererFactory() const;
    const sp<RenderEngineContext>& context() const;

    const Viewport& viewport() const;

    float toLayoutDirection(float direction) const;
    bool isLHS() const;
    bool isYUp() const;
    bool isViewportFlipped() const;

    V2 toViewportPosition(const V2& position) const;
    Rect toRendererRect(const Rect& scissor, Ark::RendererCoordinateSystem cs = Ark::COORDINATE_SYSTEM_DEFAULT) const;
    V3 toWorldPosition(const M4& vpMatrix, float screenX, float screenY, float z) const;

    void onSurfaceCreated();

    sp<RenderView> createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const;

    const PlatformInfo& info() const;
    PlatformInfo& info();

private:
    PlatformInfo _info;
    Ark::RendererCoordinateSystem _coordinate_system;

    sp<RendererFactory> _renderer_factory;
    sp<RenderEngineContext> _render_context;

};

}
