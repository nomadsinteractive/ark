#include "app/base/application_context.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/manifest.h"
#include "core/base/plugin_manager.h"
#include "core/base/string_table.h"
#include "core/base/thread.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/executor/executor_worker_thread.h"
#include "core/impl/executor/executor_thread_pool.h"
#include "core/impl/message_loop/message_loop_default.h"
#include "core/impl/runnable/runnable_by_function.h"

#include "core/inf/runnable.h"
#include "core/types/global.h"

#include "graphics/impl/vec/vec2_impl.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"

#include "app/base/application_resource.h"
#include "app/base/event.h"
#include "app/base/message_loop_thread.h"
#include "app/base/resource_loader.h"
#include "app/impl/event_listener/event_listener_list.h"

#include "platform/platform.h"


namespace ark {

namespace {

class DeferedRunnable : public Runnable {
public:
    DeferedRunnable(const sp<MessageLoop>& messageLoop, const sp<Runnable>& runnable)
        : _message_loop(messageLoop), _runnable(runnable) {
    }

    virtual void run() override {
        _message_loop->post(_runnable, 0);
    }

private:
    sp<MessageLoop> _message_loop;
    sp<Runnable> _runnable;
};

}

ApplicationContext::ApplicationContext(const sp<ApplicationResource>& applicationResources, const sp<RenderEngine>& renderEngine)
    : _ticker(sp<Ticker>::make()), _cursor_position(sp<Vec2Impl>::make()), _application_resource(applicationResources), _render_engine(renderEngine), _render_controller(sp<RenderController>::make(renderEngine, applicationResources->recycler(), applicationResources->bitmapBundle(), applicationResources->bitmapBoundsBundle())),
      _clock(sp<Clock>::make(_ticker)), _worker_strategy(sp<ExecutorWorkerStrategy>::make()), _executor_main(sp<ExecutorWorkerThread>::make(_worker_strategy)), _message_loop(makeMessageLoop()),
      _executor_pooled(sp<ExecutorThreadPool>::make(_message_loop)), _event_listeners(new EventListenerList()), _string_table(Global<StringTable>()), _background_color(Color::BLACK),
      _paused(false)
{
    Ark& ark = Ark::instance();

    for(int32_t i = 0; i < ark.argc(); i++)
        _argv.push_back(ark.argv()[i]);
}

ApplicationContext::~ApplicationContext()
{
}

void ApplicationContext::initResourceLoader(const document& manifest)
{
    const document doc = createResourceLoaderManifest(manifest);
    _resource_loader = createResourceLoaderImpl(manifest, nullptr);
    _resource_loader->import(doc, _resource_loader->beanFactory());
}

sp<ResourceLoader> ApplicationContext::createResourceLoader(const String& name, const Scope& args)
{
    Identifier id(Identifier::parse(name));
    if(id.isVal())
    {
        const document doc = _application_resource->loadDocument(name);
        DCHECK(doc, "Resource \"%s\" not found", name.c_str());
        return createResourceLoader(doc, nullptr);
    }
    return _resource_loader->beanFactory().build<ResourceLoader>(name, args);
}

sp<ResourceLoader> ApplicationContext::createResourceLoader(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    DCHECK(_resource_loader, "Application ResourceLoader has not been initialized");
    const sp<ResourceLoader> resourceLoader = createResourceLoaderImpl(manifest, resourceLoaderContext);
    resourceLoader->beanFactory().extend(_resource_loader->beanFactory());
    resourceLoader->import(manifest, _resource_loader->beanFactory());
    return resourceLoader;
}

sp<ResourceLoader> ApplicationContext::createResourceLoaderImpl(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const document doc = createResourceLoaderManifest(manifest);
    const sp<DictionaryByAttributeName> documentDictionary = sp<DictionaryByAttributeName>::make(doc, Constants::Attributes::ID);
    const sp<BeanFactory> beanFactory = Ark::instance().createBeanFactory(documentDictionary);
    const sp<ResourceLoaderContext> context = resourceLoaderContext ? resourceLoaderContext : sp<ResourceLoaderContext>::make(_application_resource->documents(), _application_resource->bitmapBundle(), _application_resource->bitmapBoundsBundle(), _executor_pooled, _render_controller);

    const Global<PluginManager> pluginManager;
    pluginManager->each([&] (const sp<Plugin>& plugin)->bool {
        plugin->loadResourceLoader(beanFactory, documentDictionary, context);
        return true;
    });

    return sp<ResourceLoader>::make(beanFactory);
}

document ApplicationContext::createResourceLoaderManifest(const document& manifest)
{
    DASSERT(manifest);
    const String src = Documents::getAttribute(manifest, Constants::Attributes::SRC);
    const document doc = src ? _application_resource->loadDocument(src) : manifest;
    DWARN(doc == manifest || manifest->children().size() == 0, "\"%s\" has already specified a ResourceLoader src \"%s\", its content will be ignored", Documents::toString(manifest).c_str(), src.c_str());
    return doc;
}

sp<MessageLoop> ApplicationContext::makeMessageLoop()
{
    _render_message_loop = sp<MessageLoopDefault>::make(_ticker);

    const Ark& ark = Ark::instance();
    if(ark.manifest()->application()._message_loop == Manifest::MESSAGE_LOOP_TYPE_RENDER)
        return _render_message_loop;
    return sp<MessageLoopThread>::make(_ticker);
}

const sp<ApplicationResource>& ApplicationContext::applicationResource() const
{
    return _application_resource;
}

const sp<RenderEngine>& ApplicationContext::renderEngine() const
{
    return _render_engine;
}

const sp<RenderController>& ApplicationContext::renderController() const
{
    return _render_controller;
}

const sp<ResourceLoader>& ApplicationContext::resourceLoader() const
{
    return _resource_loader;
}

const sp<Executor>& ApplicationContext::executorMain() const
{
    return _executor_main;
}

const sp<Executor>& ApplicationContext::executorPooled() const
{
    return _executor_pooled;
}

const std::vector<String>& ApplicationContext::argv() const
{
    return _argv;
}

const sp<Clock>& ApplicationContext::clock() const
{
    return _clock;
}

const sp<Vec2Impl>& ApplicationContext::cursorPosition() const
{
    return _cursor_position;
}

bool ApplicationContext::onEvent(const Event& event)
{
    if(event.action() == Event::ACTION_UP || event.action() == Event::ACTION_DOWN || event.action() == Event::ACTION_MOVE)
        _cursor_position->set(event.xy());
    return _event_listeners->onEvent(event) || (_default_event_listener && _default_event_listener->onEvent(event));
}

void ApplicationContext::addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& disposed)
{
    _render_controller->addPreRenderRunRequest(task, disposed);
}

void ApplicationContext::addEventListener(const sp<EventListener>& eventListener, int32_t priority)
{
    _event_listeners->addEventListener(eventListener, priority);
}

void ApplicationContext::setDefaultEventListener(const sp<EventListener>& eventListener)
{
    _default_event_listener = eventListener;
}

void ApplicationContext::post(const sp<Runnable>& task, float delay)
{
    _message_loop->post(task, delay);
}

void ApplicationContext::schedule(const sp<Runnable>& task, float interval)
{
    _message_loop->schedule(task, interval);
}

void ApplicationContext::runAtMainThread(std::function<void()> task)
{
    _message_loop->post(sp<RunnableByFunction>::make(std::move(task)), 0);
}

void ApplicationContext::addStringBundle(const String& name, const sp<StringBundle>& stringBundle)
{
    _string_table->addStringBundle(name, stringBundle);
}

sp<String> ApplicationContext::getString(const String& resid)
{
    DASSERT(resid);
    const Identifier id = resid.at(0) == '@' ? Identifier::parse(resid, Identifier::ID_TYPE_AUTO, false) : Identifier::parseRef(resid, false);
    return _string_table->getString(id.package(), id.ref(), true);
}

std::vector<String> ApplicationContext::getStringArray(const String& resid)
{
    DASSERT(resid);
    const Identifier id = resid.at(0) == '@' ? Identifier::parse(resid, Identifier::ID_TYPE_AUTO, false) : Identifier::parseRef(resid, false);
    return _string_table->getStringArray(id.package(), id.ref(), true);
}

sp<Runnable> ApplicationContext::defer(const sp<Runnable>& task) const
{
    return sp<DeferedRunnable>::make(_message_loop, task);
}

const Color& ApplicationContext::backgroundColor() const
{
    return _background_color;
}

void ApplicationContext::setBackgroundColor(const Color& backgroundColor)
{
    _background_color = backgroundColor;
}

void ApplicationContext::pause()
{
    _paused = true;
    _clock->pause();
}

void ApplicationContext::resume()
{
    _paused = false;
    _clock->resume();
}

void ApplicationContext::updateRenderState()
{
    _ticker->update(0);
    _render_message_loop->pollOnce();
}

bool ApplicationContext::isPaused() const
{
    return _paused;
}

ApplicationContext::Ticker::Ticker()
    : _steady_clock(Platform::getSteadyClock()), _val(_steady_clock->val())
{
}

bool ApplicationContext::Ticker::update(uint64_t /*timestamp*/)
{
    _val = _steady_clock->val();
    return true;
}

uint64_t ApplicationContext::Ticker::val()
{
    return _val;
}

void ApplicationContext::ExecutorWorkerStrategy::onStart()
{
}

void ApplicationContext::ExecutorWorkerStrategy::onExit()
{
}

uint64_t ApplicationContext::ExecutorWorkerStrategy::onBusy()
{
    return 0;
}

uint64_t ApplicationContext::ExecutorWorkerStrategy::onIdle(Thread& /*thread*/)
{
    return 20000;
}

void ApplicationContext::ExecutorWorkerStrategy::onException(const std::exception& e)
{
    throw e;
}

}
