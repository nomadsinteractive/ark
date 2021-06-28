#include "app/base/application.h"

#include "core/base/string_table.h"
#include "core/inf/dictionary.h"
#include "core/inf/string_bundle.h"
#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/camera.h"
#include "graphics/base/viewport.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/inf/render_view.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"

#include "app/base/application_context.h"
#include "app/base/application_delegate.h"
#include "app/base/event.h"
#include "app/base/surface.h"
#include "app/base/surface_updater.h"

#include "platform/platform.h"

namespace ark {

namespace {

class AssetStringBundle : public StringBundle {
public:
    virtual sp<String> getString(const String& name) override {
        const sp<Readable> readable = Ark::instance().openAsset(name);
        return sp<String>::make(Strings::loadFromReadable(readable));
    }

    virtual std::vector<String> getStringArray(const String& /*resid*/) override {
        return {};
    }
};

class OnSurfaceUpdatePreCreated : public Runnable {
public:
    OnSurfaceUpdatePreCreated(sp<ApplicationContext> applicationContext)
        : _application_context(std::move(applicationContext)) {
    }

    virtual void run() override {
        _application_context->updateRenderState();
    }

private:
    sp<ApplicationContext> _application_context;
};

class OnSurfaceUpdatePostCreated : public Runnable {
public:
    OnSurfaceUpdatePostCreated(sp<Runnable> runAtCore, sp<ApplicationContext> applicationContext, sp<ApplicationDelegate> applicationDelegate)
        : _run_at_core(std::move(runAtCore)), _application_context(std::move(applicationContext)), _application_delegate(std::move(applicationDelegate)) {
    }

    virtual void run() override {
        DPROFILER_TRACE("MainFrame", ApplicationProfiler::CATEGORY_RENDER_FRAME);
        _application_context->post(_run_at_core);
        _application_context->updateRenderState();
        _application_delegate->onSurfaceDraw();
    }

private:
    sp<Runnable> _run_at_core;
    sp<ApplicationContext> _application_context;
    sp<ApplicationDelegate> _application_delegate;
};

}

Application::Application(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport)
    : _application_delegate(applicationDelegate), _application_context(applicationContext), _viewport(viewport), _width(width), _height(height),
      _on_surface_update(makeOnSurfaceUpdate(false))
{
}

Application::~Application()
{
}

const char* Application::name() const
{
    return _application_delegate->name();
}

const sp<ApplicationContext>& Application::context() const
{
    return _application_context;
}

void Application::onCreateTask()
{
    __thread_init__<THREAD_ID_CORE>();
    _application_delegate->onCreate(*this, _surface);
    _on_surface_update = makeOnSurfaceUpdate(true);
}

void Application::onPauseTask()
{
    _application_delegate->onPause();
}

void Application::onResumeTask()
{
    _application_delegate->onResume();
}

void Application::onEventTask(const Event& event)
{
    DCHECK(_width && _height, "Illegal surface context state");
    _application_delegate->onEvent(event);
}

sp<Runnable> Application::makeOnSurfaceUpdate(bool alive) const
{
    if(alive)
        return sp<OnSurfaceUpdatePostCreated>::make(_surface->updater(), _application_context, _application_delegate);
    return sp<OnSurfaceUpdatePreCreated>::make(_application_context);
}

void Application::onCreate()
{
    LOGD("");
    __thread_init__<THREAD_ID_MAIN>();
    const Global<StringTable> stringTable;
    stringTable->addStringBundle("asset", sp<AssetStringBundle>::make());
    const sp<RenderView> renderView = _application_context->renderEngine()->createRenderView(_application_context->renderController(), _viewport);
    _surface = sp<Surface>::make(renderView, _application_context);
    _application_context->runAtCoreThread([this] () {
        onCreateTask();
    });
}

void Application::onPause()
{
    LOGD("");
    _on_surface_update = makeOnSurfaceUpdate(false);
    _application_context->runAtCoreThread([this] () {
        onPauseTask();
    });
    _application_context->pause();
}

void Application::onResume()
{
    LOGD("");
    _application_context->runAtCoreThread([this] () {
        onResumeTask();
        _on_surface_update = makeOnSurfaceUpdate(true);
    });
    _application_context->resume();
}

void Application::onDestroy()
{
    LOGD("");
    _on_surface_update = makeOnSurfaceUpdate(false);
    const sp<ApplicationDelegate> applicationDelegate = _application_delegate;
    const sp<ApplicationContext> applicationContext = _application_context;
    _application_context->resume();
    _application_context->runAtCoreThread([applicationDelegate, applicationContext] () {
        applicationDelegate->onDestroy();
    });
}

void Application::onSurfaceCreated()
{
    LOGD("");
    __thread_init__<THREAD_ID_RENDERER>();

    _application_context->renderEngine()->onSurfaceCreated();
    _surface->onSurfaceCreated();
    _application_delegate->onSurfaceCreated(_surface);
}

void Application::onSurfaceChanged(uint32_t width, uint32_t height)
{
    LOGD("width = %d, height = %d", width, height);
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    _application_context->runAtCoreThread([this] () {
        _application_context->renderController()->reset();
    });

    _surface->onSurfaceChanged(width, height);
    _application_delegate->onSurfaceChanged(width, height);
    _application_context->renderEngine()->context()->setDisplayResolution({width, height});
    _width = width;
    _height = height;
}

void Application::onSurfaceUpdate()
{
    _on_surface_update->run();
}

bool Application::onEvent(const Event& event)
{
    _application_context->runAtCoreThread([this, event] () {
        onEventTask(event);
    });
    return true;
}

V2 Application::toViewportPosition(const V2& xy) const
{
    return V2(_viewport.toViewportX(xy.x(), _width), _viewport.toViewportY(xy.y(), _height));
}

}
