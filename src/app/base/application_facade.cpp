#include "app/base/application_facade.h"

#include "core/base/future.h"
#include "core/components/discarded.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/base/camera.h"
#include "graphics/components/size.h"
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

class UpdatableExpecting final : public Updatable {
public:
    UpdatableExpecting(sp<Boolean> condition, sp<Future> future)
        : _condition(std::move(condition)), _future(std::move(future)) {
    }

    bool update(const uint64_t timestamp) override {
        const bool dirty = _condition->update(timestamp);
        if(_condition->val())
            _future->notify();
        return dirty;
    }

private:
    sp<Boolean> _condition;
    sp<Future> _future;
};

class FragCoordRevert final : public Vec2 {
public:
    FragCoordRevert(sp<Vec2> xy, const float height)
        : _xy(std::move(xy)), _height(height) {
    }

    bool update(const uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _xy);
    }

    V2 val() override
    {
        const V2 xy = _xy->val();
        return {xy.x(), _height - xy.y()};
    }

private:
    sp<Vec2> _xy;
    float _height;
};

class FragCoordStretch final : public Vec2 {
public:
    FragCoordStretch(sp<Vec2> xy, const V2& viewportXY, sp<Size> resolution)
        : _xy(std::move(xy)), _viewport_xy(viewportXY), _resolution(std::move(resolution)) {
    }

    bool update(const uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _xy, _resolution);
    }

    V2 val() override
    {
        const V2 xy = _xy->val();
        const V3 resolution = _resolution->val();
        return xy / V2(_viewport_xy.x(), _viewport_xy.y()) * V2(resolution.x(), resolution.y());
    }

private:
    sp<Vec2> _xy;
    V2 _viewport_xy;
    sp<Size> _resolution;
};

}

ApplicationFacade::ApplicationFacade(Application& app, const Surface& surface)
    : _context(app.context()), _controller(app.controller()), _surface_controller(surface.controller()), _surface_size(app.surfaceSize())
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
    return resolution ? sp<Vec2>::make<FragCoordStretch>(std::move(xy), V2(viewport.width(), viewport.height()), std::move(resolution)) : std::move(xy);
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
    return Ark::instance().manifest();
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

const sp<ApplicationEventListener>& ApplicationFacade::applicationEventListener() const
{
    return _context->applicationEventListener();
}

void ApplicationFacade::setApplicationEventListener(sp<ApplicationEventListener> applicationEventListener) const
{
    _context->setApplicationEventListener(std::move(applicationEventListener));
}

sp<ResourceLoader> ApplicationFacade::createResourceLoader(const String& name, const Scope& args) const
{
    return _context->createResourceLoader(name, args);
}

sp<MessageLoop> ApplicationFacade::makeMessageLoop(const sp<Clock>& clock)
{
    return _context->makeMessageLoop(clock);
}

const Vector<String>& ApplicationFacade::argv() const
{
    return _context->argv();
}

void ApplicationFacade::addPreRenderTask(sp<Runnable> task, sp<Boolean> cancelled)
{
    _context->addPreComposeRunnable(std::move(task), std::move(cancelled));
}

void ApplicationFacade::addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _context->addEventListener(std::move(eventListener), std::move(discarded));
}

void ApplicationFacade::pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _context->pushEventListener(std::move(eventListener), std::move(discarded));
}

void ApplicationFacade::exit()
{
    _controller->exit();
}

sp<Future> ApplicationFacade::post(sp<Runnable> task, const float delay, sp<Boolean> canceled) const
{
    sp<Future> future = sp<Future>::make(std::move(task), std::move(canceled));
    _context->messageLoopApp()->post(future, delay, future->isDoneOrCanceled());
    return future;
}

sp<Future> ApplicationFacade::schedule(sp<Runnable> task, const float interval, sp<Boolean> canceled, const uint32_t countDown) const
{
    sp<Future> future = sp<Future>::make(std::move(task), std::move(canceled), countDown);
    _context->messageLoopApp()->schedule(future, interval, future->isDoneOrCanceled());
    return future;
}

void ApplicationFacade::expect(sp<Boolean> condition, sp<Future> future) const
{
    _context->renderController()->addPreComposeUpdatable(sp<Updatable>::make<UpdatableExpecting>(std::move(condition), future), future->isDoneOrCanceled());
}

void ApplicationFacade::addStringBundle(const String& name, const sp<StringBundle>& stringBundle)
{
    _context->addStringBundle(name, stringBundle);
}

Optional<String> ApplicationFacade::getString(const String& resid, const Optional<String>& defValue) const
{
    if(Optional<String> val = _context->getString(resid, !static_cast<bool>(defValue)))
        return val;
    return defValue;
}

Vector<String> ApplicationFacade::getStringArray(const String& resid) const
{
    return _context->getStringArray(resid);
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

const SafeVar<Boolean>& ApplicationFacade::textInputEnabled() const
{
    return _text_input_enabled;
}

void ApplicationFacade::setTextInputEnabled(sp<Boolean> enabled)
{
    _text_input_enabled.reset(std::move(enabled));
}

}
