#include "app/base/application_facade.h"

#include "core/epi/disposed.h"

#include "graphics/base/camera.h"
#include "graphics/base/surface_controller.h"
#include "graphics/impl/vec/vec2_impl.h"

#include "app/base/application.h"
#include "app/base/application_controller.h"
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

sp<Vec2> ApplicationFacade::cursorPosition() const
{
    return _context->cursorPosition();
}

const sp<ApplicationController>& ApplicationFacade::applicationController() const
{
    return _controller;
}

const sp<SurfaceController>& ApplicationFacade::surfaceController() const
{
    return _surface_controller;
}

const sp<Camera>& ApplicationFacade::camera() const
{
    return Camera::getDefaultCamera();
}

const sp<Manifest>& ApplicationFacade::manifest() const
{
    return _manifest;
}

const sp<ResourceLoader>& ApplicationFacade::resourceLoader() const
{
    return _context->resourceLoader();
}

const sp<Arena>& ApplicationFacade::arena() const
{
    return _arena;
}

void ApplicationFacade::setArena(const sp<Arena>& arena)
{
    if(_arena == arena)
    {
        DWARN(false, "Replacing current Arena with the same one");
        return;
    }

    if(_arena)
        _arena.as<Disposed>()->dispose();

    DASSERT(arena);
    DWARN(!arena.is<Disposed>(), "Application main arena's lifecycle should be managed by application itself");
    _arena = arena;
    _arena.absorb<Disposed>(sp<Disposed>::make());

    _surface_controller->addRenderer(_arena);
    _context->addEventListener(_arena, 0);
}

sp<ResourceLoader> ApplicationFacade::createResourceLoader(const String& name, const Scope& args)
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

void ApplicationFacade::addEventListener(const sp<EventListener>& eventListener, int32_t priority)
{
    _context->addEventListener(eventListener, priority);
}

void ApplicationFacade::setDefaultEventListener(const sp<EventListener>& eventListener)
{
    _context->setDefaultEventListener(eventListener);
}

void ApplicationFacade::exit()
{
    _controller->exit();
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

std::vector<String> ApplicationFacade::getStringArray(const String& resid)
{
    return _context->getStringArray(resid);
}

sp<Runnable> ApplicationFacade::defer(const sp<Runnable>& task) const
{
    return _context->defer(task);
}

sp<Numeric> ApplicationFacade::synchronize(const sp<Numeric>& value) const
{
    return _context->synchronize(value);
}

sp<Vec2> ApplicationFacade::synchronize(const sp<Vec2>& value) const
{
    return _context->synchronize(value);
}

sp<Vec3> ApplicationFacade::synchronize(const sp<Vec3>& value) const
{
    return _context->synchronize(value);
}

sp<Vec4> ApplicationFacade::synchronize(const sp<Vec4>& value) const
{
    return _context->synchronize(value);
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
