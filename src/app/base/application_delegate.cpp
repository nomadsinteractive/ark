#include "app/base/application_delegate.h"

#include "core/ark.h"

#include "graphics/inf/render_view.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/surface.h"

#include "platform/platform.h"
#include "platform/gl/gl.h"

namespace ark {

ApplicationDelegate::ApplicationDelegate(const Color& backgroundColor)
    : _background_color(backgroundColor)
{
}

ApplicationDelegate::~ApplicationDelegate()
{
}

void ApplicationDelegate::onCreate(Application& application, const sp<Surface>& surface)
{
    _application_context = application.context();
    _surface = surface;
}

void ApplicationDelegate::onPause()
{
}

void ApplicationDelegate::onResume()
{
}

void ApplicationDelegate::onDestroy()
{
    _render_view = nullptr;
    _application_context = nullptr;
}

void ApplicationDelegate::onSurfaceCreated(const sp<Surface>& surface)
{
    _render_view = surface->renderView();
}

void ApplicationDelegate::onSurfaceChanged(uint32_t /*width*/, uint32_t /*height*/)
{
}

void ApplicationDelegate::onSurfaceDraw()
{
    glClearColor(_background_color.red(), _background_color.green(), _background_color.blue(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _surface->onRenderFrame();
}

bool ApplicationDelegate::onEvent(const Event& event)
{
    return _application_context->onEvent(event);
}

}
