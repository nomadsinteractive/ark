#include "app/base/application_delegate.h"

#include "graphics/inf/render_view.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/application_manifest.h"
#include "app/base/surface.h"
#include "core/inf/interpreter.h"

namespace ark {

void ApplicationDelegate::onCreate(Application& application, const sp<Surface>& surface)
{
    _application_context = application.context();
    _application_context->interpreter()->initialize();
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
    _surface->onRenderFrame(_application_context->backgroundColor());
}

bool ApplicationDelegate::onEvent(const Event& event)
{
    return _application_context->onEvent(event);
}

}
