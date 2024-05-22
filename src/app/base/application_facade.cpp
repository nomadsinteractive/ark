#include "app/base/application_facade.h"

#include "core/base/future.h"
#include "core/traits/disposed.h"

#include "graphics/base/camera.h"
#include "graphics/base/surface_controller.h"
#include "graphics/impl/vec/vec2_impl.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/surface.h"
#include "app/inf/application_controller.h"
#include "app/view/arena.h"

namespace ark {

namespace {

class UpdatableExpecting : public Updatable {
public:
    UpdatableExpecting(sp<Boolean> condition, sp<Future> future)
        : _condition(std::move(condition)), _future(std::move(future)) {
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = _condition->update(timestamp);
        if(_condition->val())
            _future->done();
        return dirty;
    }

private:
    sp<Boolean> _condition;
    sp<Future> _future;
};

}

ApplicationFacade::ApplicationFacade(Application& app, const Surface& surface, sp<ApplicationManifest> manifest)
    : _context(app.context()), _controller(app.controller()), _surface_controller(surface.controller()), _surface_size(app.surfaceSize()), _manifest(std::move(manifest))
{
}

const sp<Clock>& ApplicationFacade::clock() const
{
    return _context->appClock();
}

sp<Numeric> ApplicationFacade::clockInterval() const
{
    return _context->appClockInterval();
}

sp<Vec2> ApplicationFacade::cursorPosition() const
{
    return _context->cursorPosition();
}

const sp<Size>& ApplicationFacade::surfaceSize() const
{
    return _surface_size;
}

const sp<ApplicationController>& ApplicationFacade::applicationController() const
{
    return _controller;
}

const sp<ApplicationBundle>& ApplicationFacade::applicationBundle() const
{
    return _context->applicationBundle();
}

const sp<RenderController>& ApplicationFacade::renderController() const
{
    return _context->renderController();
}

const sp<SurfaceController>& ApplicationFacade::surfaceController() const
{
    return _surface_controller;
}

const sp<Camera>& ApplicationFacade::camera() const
{
    return Camera::getDefaultCamera();
}

const sp<ApplicationManifest>& ApplicationFacade::manifest() const
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

void ApplicationFacade::setArena(sp<Arena> arena)
{
    if(_arena == arena)
    {
        WARN("Replacing current Arena with the same one");
        return;
    }

    if(_arena)
    {
        _arena_discarded->dispose();
        _context->deferUnref(std::move(_arena));
    }

    ASSERT(arena);
    _arena = std::move(arena);
    _arena_discarded = sp<Disposed>::make();

    _surface_controller->addRenderer(_arena, _arena_discarded);
    _context->addEventListener(_arena, _arena_discarded);
}

sp<ResourceLoader> ApplicationFacade::createResourceLoader(const String& name, const Scope& args)
{
    return _context->createResourceLoader(name, args);
}

sp<MessageLoop> ApplicationFacade::makeMessageLoop(const sp<Clock>& clock)
{
    return _context->makeMessageLoop(clock);
}

const std::vector<String>& ApplicationFacade::argv() const
{
    return _context->argv();
}

void ApplicationFacade::addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& disposed)
{
    _context->addPreRenderTask(task, disposed);
}

void ApplicationFacade::addControlLayer(const sp<Renderer>& controlLayer)
{
    _surface_controller->addControlLayer(controlLayer);
}

void ApplicationFacade::addEventListener(sp<EventListener> eventListener, sp<Boolean> disposed)
{
    _context->addEventListener(std::move(eventListener), std::move(disposed));
}

void ApplicationFacade::pushEventListener(sp<EventListener> eventListener, sp<Boolean> disposed)
{
    _context->pushEventListener(std::move(eventListener), std::move(disposed));
}

void ApplicationFacade::setDefaultEventListener(sp<EventListener> eventListener)
{
    _context->setDefaultEventListener(std::move(eventListener));
}

void ApplicationFacade::exit()
{
    _controller->exit();
}

void ApplicationFacade::post(sp<Runnable> task, float delay, sp<Boolean> canceled)
{
    _context->messageLoopApp()->post(std::move(task), delay, std::move(canceled));
}

void ApplicationFacade::post(sp<Runnable> task, const std::vector<float>& delays, sp<Boolean> canceled)
{
    for(float i : delays)
        post(task, i, canceled);
}

void ApplicationFacade::schedule(sp<Runnable> task, float interval, sp<Boolean> canceled)
{
    _context->messageLoopApp()->schedule(std::move(task), interval, std::move(canceled));
}

sp<Future> ApplicationFacade::expect(sp<Boolean> condition, sp<Observer> observer, sp<Boolean> canceled)
{
    sp<Future> future = sp<Future>::make(std::move(canceled), std::move(observer));
    _context->renderController()->addPreComposeUpdatable(sp<UpdatableExpecting>::make(std::move(condition), future), future->canceled());
    return future;
}

void ApplicationFacade::addStringBundle(const String& name, const sp<StringBundle>& stringBundle)
{
    _context->addStringBundle(name, stringBundle);
}

sp<String> ApplicationFacade::getString(const String& resid,  const sp<String>& defValue)
{
    sp<String> val = _context->getString(resid, !static_cast<bool>(defValue));
    return val ? val : defValue;
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
