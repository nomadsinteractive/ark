#include "app/base/application_facade.h"

#include "core/base/future.h"
#include "core/traits/discarded.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/base/camera.h"
#include "graphics/traits/size.h"
#include "graphics/base/surface_controller.h"
#include "graphics/impl/vec/vec2_impl.h"

#include "renderer/base/render_engine.h"
#include "renderer/inf/renderer_factory.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/surface.h"
#include "app/inf/application_controller.h"
#include "app/base/activity.h"

namespace ark {

namespace {

struct UpdatableExpecting final : Updatable {
    UpdatableExpecting(sp<Boolean> condition, sp<Future> future)
        : _condition(std::move(condition)), _future(std::move(future)) {
    }

    bool update(uint64_t timestamp) override {
        const bool dirty = _condition->update(timestamp);
        if(_condition->val())
            _future->done();
        return dirty;
    }

    sp<Boolean> _condition;
    sp<Future> _future;
};

struct FragCoordRevert final : Vec2 {
    FragCoordRevert(sp<Vec2> xy, float height)
        : _xy(std::move(xy)), _height(height) {
    }

    bool update(uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _xy);
    }

    V2 val() override
    {
        const V2 xy = _xy->val();
        return {xy.x(), _height - xy.y()};
    }

    sp<Vec2> _xy;
    float _height;
};

struct FragCoordStretch final : Vec2 {
    FragCoordStretch(sp<Vec2> xy, const V2& viewportXY, sp<Size> resolution)
        : _xy(std::move(xy)), _viewport_xy(viewportXY), _resolution(std::move(resolution)) {
    }

    bool update(uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _xy, _resolution);
    }

    V2 val() override
    {
        const V2 xy = _xy->val();
        const V3 resolution = _resolution->val();
        return xy / V2(_viewport_xy.x(), _viewport_xy.y()) * V2(resolution.x(), resolution.y());
    }

    sp<Vec2> _xy;
    V2 _viewport_xy;
    sp<Size> _resolution;
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

sp<Vec2> ApplicationFacade::toFragCoord(sp<Vec2> xy, sp<Size> resolution) const
{
    const RenderEngine& renderEngine = _context->renderController()->renderEngine();
    const Viewport& viewport = renderEngine.viewport();
    if(renderEngine.isViewportFlipped())
        xy = sp<Vec2>::make<FragCoordRevert>(std::move(xy), viewport.height());
    return resolution ? sp<Vec2>::make<FragCoordStretch>(std::move(xy), V2(viewport.width(), viewport.height()), std::move(resolution)) : xy;
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

sp<Camera> ApplicationFacade::camera() const
{
    return Global<Camera>();
}

const sp<ApplicationManifest>& ApplicationFacade::manifest() const
{
    return _manifest;
}

const sp<ResourceLoader>& ApplicationFacade::resourceLoader() const
{
    return _context->resourceLoader();
}

const sp<Activity>& ApplicationFacade::activity() const
{
    return _activity;
}

void ApplicationFacade::setActivity(sp<Activity> activity)
{
    if(_activity == activity)
    {
        WARN("Replacing current Activity with the same one");
        return;
    }

    if(_activity)
    {
        _activity_discarded->discard();
        _context->deferUnref(std::move(_activity));
    }

    ASSERT(activity);
    _activity = std::move(activity);
    _activity_discarded = sp<Discarded>::make();

    _surface_controller->addRenderer(_activity, _activity_discarded);
    _context->addEventListener(_activity, _activity_discarded);
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

void ApplicationFacade::addPreRenderTask(sp<Runnable> task, sp<Boolean> cancelled)
{
    _context->addPreRenderTask(std::move(task), std::move(cancelled));
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

void ApplicationFacade::post(sp<Runnable> task, const std::vector<float>& delays, const sp<Boolean>& canceled)
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
