#include "app/base/application_facade.h"

#include "core/epi/expired.h"

#include "graphics/base/camera.h"
#include "graphics/base/surface_controller.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/surface.h"
#include "app/view/arena.h"

namespace ark {

ApplicationFacade::ApplicationFacade(Application& app, const Surface& surface, const sp<ApplicationManifest>& manifest)
    : _context(app.context()), _controller(app.controller()), _surface_controller(surface.controller()), _manifest(manifest)
{
}

const sp<Clock>& ApplicationFacade::clock() const
{
    return _context->clock();
}

const sp<ApplicationContext>& ApplicationFacade::context() const
{
    return _context;
}

const sp<ApplicationController>& ApplicationFacade::controller() const
{
    return _controller;
}

const sp<Camera>& ApplicationFacade::camera() const
{
    return Camera::getMainCamera();
}

const sp<ApplicationManifest>& ApplicationFacade::manifest() const
{
    return _manifest;
}

const sp<Arena>& ApplicationFacade::arena() const
{
    return _arena;
}

void ApplicationFacade::setArena(const sp<Arena>& arena)
{
    if(_arena)
        _arena.as<Expired>()->expire();

    NOT_NULL(arena);
    DWARN(!arena.is<Expired>(), "Application main arena's lifecycle should be managed by application itself");
    _arena = arena;
    _arena.absorb<Expired>(sp<Expired>::make());

    _surface_controller->addRenderer(_arena);
    _context->addEventListener(_arena);
}

const Color& ApplicationFacade::backgroundColor() const
{
    return _context->backgroundColor();
}

void ApplicationFacade::setBackgroundColor(const Color& backgroundColor)
{
    _context->setBackgroundColor(backgroundColor);
}

}
