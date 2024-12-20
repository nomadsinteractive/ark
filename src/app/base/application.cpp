#include "app/base/application.h"

#include "core/base/string_table.h"
#include "core/inf/string_bundle.h"
#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/camera.h"
#include "graphics/base/size.h"
#include "graphics/base/viewport.h"
#include "graphics/inf/render_view.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"

#include "app/base/application_context.h"
#include "app/base/application_delegate.h"
#include "app/base/event.h"
#include "app/base/surface.h"
#include "app/base/surface_updater.h"

namespace ark {

namespace {

class AssetStringBundle final : public StringBundle {
public:
    sp<String> getString(const String& name) override {
        const sp<Readable> readable = Ark::instance().openAsset(name);
        return sp<String>::make(Strings::loadFromReadable(readable));
    }

    std::vector<String> getStringArray(const String& /*resid*/) override {
        return {};
    }
};

class OnSurfaceUpdatePreCreated final : public Runnable {
public:
    OnSurfaceUpdatePreCreated(sp<ApplicationContext> applicationContext)
        : _application_context(std::move(applicationContext)) {
    }

    void run() override {
        _application_context->updateRenderState();
    }

private:
    sp<ApplicationContext> _application_context;
};

class OnSurfaceUpdatePostCreated final : public Runnable {
public:
    OnSurfaceUpdatePostCreated(sp<Runnable> runAtCore, sp<ApplicationContext> applicationContext, sp<ApplicationDelegate> applicationDelegate)
        : _run_at_core(std::move(runAtCore)), _application_context(std::move(applicationContext)), _application_delegate(std::move(applicationDelegate)) {
    }

    void run() override {
        DPROFILER_TRACE("MainFrame", ApplicationProfiler::CATEGORY_RENDER_FRAME);
        _application_context->runAtCoreThread(_run_at_core);
        _application_context->updateRenderState();
        _application_delegate->onSurfaceDraw();
    }

private:
    sp<Runnable> _run_at_core;
    sp<ApplicationContext> _application_context;
    sp<ApplicationDelegate> _application_delegate;
};

}

Application::Application(sp<ApplicationDelegate> applicationDelegate, sp<ApplicationContext> applicationContext, uint32_t surfaceWidth, uint32_t surfaceHeight, const Viewport& viewport)
    : _application_delegate(std::move(applicationDelegate)), _application_context(std::move(applicationContext)), _viewport(viewport), _surface_size(sp<Size>::make(static_cast<float>(surfaceWidth), static_cast<float>(surfaceHeight))),
      _surface_updater_pre_created(sp<OnSurfaceUpdatePreCreated>::make(_application_context)), _surface_updater(_surface_updater_pre_created.get())
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

const sp<Size>& Application::surfaceSize() const
{
    return _surface_size;
}

void Application::onCreateTask()
{
    __thread_init__<THREAD_ID_CORE>();
    _application_delegate->onCreate(*this, _surface);
    setSurfaceUpdater(true);
}

void Application::onPauseTask() const
{
    _application_delegate->onPause();
}

void Application::onResumeTask() const
{
    _application_delegate->onResume();
}

void Application::onEventTask(const Event& event) const
{
    _application_delegate->onEvent(event);
}

void Application::setSurfaceUpdater(bool alive)
{
    _surface_updater = alive ? _surface_updater_created.get() : _surface_updater_pre_created.get();
}

void Application::onCreate()
{
    LOGD("");
    __thread_init__<THREAD_ID_MAIN>();
    const Global<StringTable> stringTable;
    stringTable->addStringBundle("asset", sp<AssetStringBundle>::make());
    sp<RenderView> renderView = _application_context->renderEngine()->createRenderView(_application_context->renderController(), _viewport);
    _surface = sp<Surface>::make(std::move(renderView), _application_context);
    _surface_updater_created = sp<OnSurfaceUpdatePostCreated>::make(_surface->updater(), _application_context, _application_delegate);
    _application_context->runAtCoreThread([this] () {
        onCreateTask();
    });
}

void Application::onPause()
{
    LOGD("");
    setSurfaceUpdater(false);
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
        setSurfaceUpdater(true);
    });
    _application_context->resume();
}

void Application::onDestroy()
{
    LOGD("");
    setSurfaceUpdater(false);
    const sp<ApplicationDelegate> applicationDelegate = _application_delegate;
    const sp<ApplicationContext> applicationContext = _application_context;
    _application_context->resume();
    _application_context->runAtCoreThread([applicationDelegate] () {
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

    _surface_size->setWidth(static_cast<float>(width));
    _surface_size->setHeight(static_cast<float>(height));

    _application_context->runAtCoreThread([this] () {
        _application_context->renderController()->reset();
    });

    _surface->onSurfaceChanged(width, height);
    _application_delegate->onSurfaceChanged(width, height);
    _application_context->renderEngine()->context()->setDisplayResolution({width, height});
}

void Application::onSurfaceUpdate()
{
    _surface_updater->run();
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
    return V2(_viewport.toViewportX(xy.x(), _surface_size->widthAsFloat()), _viewport.toViewportY(xy.y(), _surface_size->heightAsFloat()));
}

}
