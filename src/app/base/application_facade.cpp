#include "app/base/application_facade.h"

#include "core/epi/disposable.h"

#include "graphics/base/camera.h"
#include "graphics/base/surface_controller.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/surface.h"
#include "app/view/arena.h"

namespace ark {

ApplicationFacade::ApplicationFacade(Application& app, const Surface& surface, const sp<Manifest>& manifest)
    : _context(app.context()), _controller(app.controller()), _surface_controller(surface.controller()), _manifest(manifest)
{
}

const sp<Clock>& ApplicationFacade::clock() const
{
    return _context->clock();
}

const sp<ApplicationController>& ApplicationFacade::controller() const
{
    return _controller;
}

const sp<Camera>& ApplicationFacade::camera() const
{
    return Camera::getMainCamera();
}

const sp<Manifest>& ApplicationFacade::manifest() const
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
        _arena.as<Disposable>()->dispose();

    DASSERT(arena);
    DWARN(!arena.is<Disposable>(), "Application main arena's lifecycle should be managed by application itself");
    _arena = arena;
    _arena.absorb<Disposable>(sp<Disposable>::make());

    _surface_controller->addRenderer(_arena);
    _context->addEventListener(_arena);
}

sp<ResourceLoader> ApplicationFacade::createResourceLoader(const String& name, const sp<Scope>& args)
{
    return _context->createResourceLoader(name, args);
}

const std::vector<String>& ApplicationFacade::argv() const
{
    return _context->argv();
}

void ApplicationFacade::addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& expired)
{
    _context->addPreRenderTask(task, expired);
}

void ApplicationFacade::addEventListener(const sp<EventListener>& eventListener)
{
    _context->addEventListener(eventListener);
}

void ApplicationFacade::setDefaultEventListener(const sp<EventListener>& eventListener)
{
    _context->setDefaultEventListener(eventListener);
}

void ApplicationFacade::post(const sp<Runnable>& task, float delay)
{
    _context->post(task, delay);
}

void ApplicationFacade::schedule(const sp<Runnable>& task, float interval)
{
    _context->schedule(task, interval);
}

void ApplicationFacade::addStringBundle(const String& name, const sp<StringBundle>& stringBundle)
{
    _context->addStringBundle(name, stringBundle);
}

sp<String> ApplicationFacade::getString(const String& resid)
{
    return _context->getString(resid);
}

sp<Runnable> ApplicationFacade::defer(const sp<Runnable>& task) const
{
    return _context->defer(task);
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
