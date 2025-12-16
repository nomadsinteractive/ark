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

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/base/application_manifest.h"

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
    RenderEngine(const ApplicationManifest::Renderer& renderer, sp<RendererFactory> rendererFactory);

    enums::RendererVersion version() const;
    enums::CoordinateSystem viewportCoordinateSystem() const;
    enums::CoordinateSystem ndcCoordinateSystem() const;

    const sp<RendererFactory>& rendererFactory() const;
    const sp<RenderEngineContext>& context() const;

    const Viewport& viewport() const;

    float toLayoutDirection(float direction) const;
    bool isLHS() const;
    bool isBackendLHS() const;
    bool isViewportFlipped() const;
    bool shouldFlipY() const;

    V2 toNDC(float viewportX, float viewportY) const;
    V2 toLHSPosition(const V2& position) const;

    Rect toRendererRect(const Rect& scissor, enums::CoordinateSystem cs = enums::COORDINATE_SYSTEM_DEFAULT) const;

    void onSurfaceCreated();

    sp<RenderView> createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const;
    sp<Pipeline> createPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, std::map<enums::ShaderStageBit, String> stages);

    const PlatformInfo& info() const;
    PlatformInfo& info();

private:
    sp<RenderEngineContext> _render_context;
    sp<RendererFactory> _renderer_factory;
    sp<PipelineFactory> _pipeline_factory;
    enums::CoordinateSystem _coordinate_system;
    PlatformInfo _info;
};

}
