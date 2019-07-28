
#include <stdio.h>

#include "core/forwarding.h"
#include "core/ark.h"
#include "core/base/manifest.h"
#include "core/types/shared_ptr.h"
#include "core/util/log.h"

#include "graphics/base/size.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_context.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/application_controller.h"
#include "app/base/application_delegate_impl.h"
#include "app/base/event.h"

extern "C" {
    void ark_runtime_on_create();
    void ark_runtime_on_pause();
    void ark_runtime_on_resume();
    void ark_runtime_on_surface_created(void* view);
    void ark_runtime_on_surface_changed(int32_t surfaceWidth, int32_t surfaceHeight, int32_t viewWidth, int32_t viewHeight);
    void ark_runtime_on_surface_draw();
    void ark_runtime_on_touch(uint32_t type, float x, float y, uint32_t timestamp, uint32_t orgBottomLeft);
    void ark_runtime_get_render_resolution(float* width, float* height);
}

using namespace ark;

namespace {
    
    class DarwinApplicationController : public ApplicationController {
    public:
        DarwinApplicationController() {
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
    
    class DarwinApplication : public Application {
    public:
        DarwinApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport)
            : Application(applicationDelegate, applicationContext, width, height, viewport), _controller(sp<DarwinApplicationController>::make()) {
        }
        
        virtual int run() {
            return 0;
        }
        
        virtual const sp<ApplicationController>& controller() {
            return _controller;
        }
        
    private:
        sp<ApplicationController> _controller;
        
    };
    
}

static sp<Ark> _ark;
static sp<Application> _application;
static sp<Manifest> _manifest;
static float _event_x_ratio, _event_y_ratio;
static int32_t _view_height;

void ark_runtime_on_create()
{
    _manifest = sp<Manifest>::make("manifest.xml");
    _ark = sp<Ark>::make(0, nullptr, _manifest);
    const sp<Size>& renderResolution = _manifest->rendererResolution();
    const Viewport viewport(0.0f, 0.0f, renderResolution->width(), renderResolution->height(), 0.1f, renderResolution->width());
    _application = sp<DarwinApplication>::make(sp<ApplicationDelegateImpl>::make(_manifest), _ark->applicationContext(), (int32_t) (renderResolution->width()), (int32_t) (renderResolution->height()), viewport);
    _application->onCreate();
}

void ark_runtime_on_surface_created(void* view)
{
    RenderContext::Info& info = _application->context()->renderEngine()->renderContext()->info();
    info.darwin.view = view;
    _application->onSurfaceCreated();
    _ark->applicationContext()->updateRenderState();
}

void ark_runtime_on_surface_changed(int32_t surfaceWidth, int32_t surfaceHeight, int32_t viewWidth, int32_t viewHeight)
{
    const sp<Size>& renderResolution = _manifest->rendererResolution();
    _application->onSurfaceChanged(surfaceWidth, surfaceHeight);
    _event_x_ratio = renderResolution->width() / viewWidth;
    _event_y_ratio = renderResolution->height() / viewHeight;
    _view_height = viewHeight;
}

void ark_runtime_on_surface_draw()
{
    _application->onSurfaceUpdate();
}

void ark_runtime_on_resume()
{
    _application->onResume();
}

void ark_runtime_on_pause()
{
    _application->onPause();
}

void ark_runtime_on_touch(uint32_t type, float x, float y, uint32_t timestamp, uint32_t orgBottomLeft)
{
    DASSERT(type < 4);
    const Event::Action actionTypes[4] = {Event::ACTION_DOWN, Event::ACTION_UP, Event::ACTION_MOVE, Event::ACTION_CANCEL};
    const Event event(actionTypes[type], x * _event_x_ratio, (orgBottomLeft ? y : _view_height - y) * _event_y_ratio, timestamp);
    _application->onEvent(event, false);
}

void ark_runtime_get_render_resolution(float* width, float* height)
{
    DASSERT(_ark);
    const sp<Size>& renderResolution = Ark::instance().manifest()->rendererResolution();
    *width = renderResolution->width();
    *height = renderResolution->height();
}
