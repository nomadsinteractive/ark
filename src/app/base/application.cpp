#include "app/base/application.h"

#include "core/base/string_table.h"
#include "core/inf/dictionary.h"
#include "core/inf/runnable.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/camera.h"
#include "graphics/base/viewport.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/inf/render_view.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/application_delegate.h"
#include "app/base/event.h"
#include "app/base/surface.h"

#include "platform/platform.h"

namespace ark {

namespace {

class AssetStringBundle : public StringBundle {
public:
    virtual sp<String> get(const String& name) override {
        const sp<Readable> readable = Ark::instance().getResource(name);
        DCHECK(readable, "Cannot load resource \"%s\"", name.c_str());
        return sp<String>::make(Strings::loadFromReadable(readable));
    }
};

}

Application::Application(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport)
    : _application_delegate(applicationDelegate), _application_context(applicationContext)
      , _viewport(viewport), _width(width), _height(height), _alive(false)
{
    g_upDirection = _viewport.top() < _viewport.bottom() ? 1.0f : -1.0f;
    Camera::getMainCamera()->ortho(viewport.left(), viewport.right(), viewport.top(), viewport.bottom(), viewport.near(), viewport.far());
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
    _alive = true;
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

void Application::onCreate()
{
    LOGD("");
    __thread_init__<THREAD_ID_MAIN>();
    const Global<StringTable> stringTable;
    stringTable->addStringBundle("asset", sp<AssetStringBundle>::make());
    const sp<RenderView> renderView = _application_context->renderEngine()->createRenderView(_viewport);
    _surface = sp<Surface>::make(renderView);
    _application_context->postTask([this] () {
        onCreateTask();
    });
}

void Application::onPause()
{
    LOGD("");
    DTHREAD_CHECK(THREAD_ID_MAIN);
    _alive = false;
    _application_context->postTask([this] () {
        onPauseTask();
        _application_context->pause();
    });
}

void Application::onResume()
{
    LOGD("");
    DTHREAD_CHECK(THREAD_ID_MAIN);
    _application_context->resume();
    _application_context->postTask([this] () {
        onResumeTask();
        _alive = true;
    });
}

void Application::onDestroy()
{
    LOGD("");
    DTHREAD_CHECK(THREAD_ID_MAIN);
    _alive = false;
    const sp<ApplicationDelegate> applicationDelegate = _application_delegate;
    const sp<ApplicationContext> applicationContext = _application_context;
    _application_context->resume();
    _application_context->postTask([applicationDelegate, applicationContext] () {
        applicationDelegate->onDestroy();
        applicationContext->dispose();
    });
    _application_context->waitForFinish();
}

void Application::onSurfaceCreated()
{
    LOGD("");
    __thread_init__<THREAD_ID_RENDERER>();
    _surface->onSurfaceCreated();
    _application_delegate->onSurfaceCreated(_surface);
}

void Application::onSurfaceChanged(uint32_t width, uint32_t height)
{
    LOGD("width = %d, height = %d", width, height);
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _application_context->renderEngine()->initialize();

    _surface->onSurfaceChanged(width, height);
    _application_delegate->onSurfaceChanged(width, height);
    _width = width;
    _height = height;
}

void Application::onSurfaceDraw()
{
    if(_alive)
        _application_delegate->onSurfaceDraw();
}

bool Application::onEvent(const Event& event, bool mapViewport)
{
    const Event mapped(event.action(), mapViewport ? _viewport.toViewportX(event.x(), _width) : event.x(),
                       mapViewport ? _viewport.toViewportY(event.y(), _height) : event.y(), event.timestamp(), event.code());
    _application_context->postTask([this, mapped] () {
        onEventTask(mapped);
    });
    return true;
}

}
