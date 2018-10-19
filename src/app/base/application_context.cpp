#include "app/base/application_context.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/string_table.h"
#include "core/base/thread.h"
#include "core/base/thread_pool_executor.h"
#include "core/base/plugin_manager.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/dictionary_impl.h"
#include "core/impl/message_loop/message_loop_default.h"
#include "core/inf/variable.h"
#include "core/inf/runnable.h"
#include "core/types/global.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/render_view_factory.h"

#include "app/base/application_resource.h"
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

class ApplicationContext::EngineTicker : public Variable<uint64_t>, public Runnable {
public:
    EngineTicker();

    virtual uint64_t val() override;
    virtual void run() override;

private:
    sp<Variable<uint64_t>> _ticker;
    uint64_t _tick;
};

ApplicationContext::ApplicationContext(const sp<ApplicationResource>& applicationResources)
    : _application_resource(applicationResources), _ticker(sp<EngineTicker>::make()),
      _clock(sp<Clock>::make(_ticker)), _message_loop_application(sp<MessageLoopThread>::make(sp<MessageLoopDefault>::make(Platform::getSteadyClock()))),
      _executor(sp<ThreadPoolExecutor>::make(_message_loop_application)), _render_controller(sp<RenderController>::make()),
      _event_listeners(new EventListenerList()), _string_table(Global<StringTable>()), _background_color(Color::BLACK)
{
    Ark& ark = Ark::instance();

    for(int32_t i = 0; i < ark.argc(); i++)
        _argv.push_back(ark.argv()[i]);

    _render_controller->addPreUpdateRequest(_ticker, sp<BooleanByWeakRef<EngineTicker>>::make(_ticker, 1));
    initMessageLoop();
}

ApplicationContext::~ApplicationContext()
{
    _message_loop_application->terminate();
}

void ApplicationContext::initMessageLoop()
{
    _message_loop_application->start();
    Ark::instance().put<MessageLoop>(_message_loop_application);
}

void ApplicationContext::initResourceLoader(const document& manifest)
{
    const sp<DictionaryByAttributeName> documentDictionary = sp<DictionaryByAttributeName>::make(manifest, Constants::Attributes::ID);
    _resource_loader = createResourceLoader(documentDictionary, nullptr);
    _resource_loader->import(manifest, _resource_loader->beanFactory());
}

sp<ResourceLoader> ApplicationContext::createResourceLoader(const String& name, const sp<Scope>& args)
{
    DCHECK(_resource_loader, "Application ResourceLoader has not been initialized.");
    return _resource_loader->beanFactory().ensure<ResourceLoader>(name, args);
}

sp<ResourceLoader> ApplicationContext::createResourceLoader(const String& name, const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Scope>& args)
{
    const document doc = name ? _application_resource->loadDocument(name) : sp<Document>::make("");
    DCHECK(doc, "Resource \"%s\" not found", name.c_str());
    const sp<DictionaryByAttributeName> documentDictionary = sp<DictionaryByAttributeName>::make(doc, Constants::Attributes::ID);
    const sp<ResourceLoader> resourceLoader = createResourceLoader(documentDictionary, resourceLoaderContext);
    resourceLoader->import(doc, resourceLoader->beanFactory());
    return resourceLoader;
}

sp<ResourceLoader> ApplicationContext::createResourceLoader(const sp<Dictionary<document>>& documentDictionary, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const sp<BeanFactory> beanFactory = Ark::instance().createBeanFactory(documentDictionary);
    const sp<GLResourceManager> glResourceManager = _application_resource->glResourceManager();
    const sp<ResourceLoaderContext> context = resourceLoaderContext ? resourceLoaderContext : sp<ResourceLoaderContext>::make(_application_resource->documents(), _application_resource->imageResource(), glResourceManager, _executor, _render_controller);
    const Global<PluginManager> pluginManager;
    pluginManager->each([&] (const sp<Plugin>& plugin)->bool {
        plugin->loadResourceLoader(beanFactory, documentDictionary, context);
        return true;
    });
    if(_resource_loader)
        beanFactory->extend(_resource_loader->beanFactory());
    return sp<ResourceLoader>::make(beanFactory);
}

const sp<ApplicationResource>& ApplicationContext::applicationResource() const
{
    return _application_resource;
}

const sp<GLResourceManager>& ApplicationContext::glResourceManager() const
{
    return _application_resource->glResourceManager();
}

const sp<RenderController>& ApplicationContext::renderController() const
{
    return _render_controller;
}

const sp<Executor>& ApplicationContext::executor() const
{
    return _executor;
}

const List<String>& ApplicationContext::argv() const
{
    return _argv;
}

const sp<Clock>& ApplicationContext::clock() const
{
    return _clock;
}

bool ApplicationContext::onEvent(const Event& event)
{
    return _event_listeners->onEvent(event) || (_default_event_listener && _default_event_listener->onEvent(event));
}

void ApplicationContext::addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& expired)
{
    _render_controller->addPreUpdateRequest(task, expired);
}

void ApplicationContext::addEventListener(const sp<EventListener>& eventListener)
{
    _event_listeners->addEventListener(eventListener);
}

void ApplicationContext::setDefaultEventListener(const sp<EventListener>& eventListener)
{
    _default_event_listener = eventListener;
}

void ApplicationContext::post(const sp<Runnable>& task, float delay)
{
    _message_loop_application->post(task, delay);
}

void ApplicationContext::schedule(const sp<Runnable>& task, float interval)
{
    _message_loop_application->schedule(task, interval);
}

uint64_t ApplicationContext::pollOnce()
{
    return _message_loop_application->pollOnce();
}

void ApplicationContext::addStringBundle(const String& name, const sp<StringBundle>& stringBundle)
{
    _string_table->addStringBundle(name, stringBundle);
}

sp<String> ApplicationContext::getString(const String& resid)
{
    DASSERT(resid);
    const Identifier id = resid.at(0) == '@' ? Identifier::parse(resid, Identifier::FORMAT_URL) : Identifier::parseRef(resid);
    return _string_table->getString(id.package(), id.ref());
}

sp<Runnable> ApplicationContext::defer(const sp<Runnable>& task) const
{
    return sp<DeferedRunnable>::make(_message_loop_application, task);
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
    if(_message_loop_application->thread().isRunning())
        _message_loop_application->pause();
}

void ApplicationContext::resume()
{
    if(_message_loop_application->thread().isPaused())
        _message_loop_application->resume();
}

void ApplicationContext::dispose()
{
    _message_loop_application->terminate();
}

void ApplicationContext::waitForFinish()
{
    _message_loop_application->join();
}

ApplicationContext::EngineTicker::EngineTicker()
    : _ticker(Platform::getSteadyClock()), _tick(0)
{
}

uint64_t ApplicationContext::EngineTicker::val()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    return _tick;
}

void ApplicationContext::EngineTicker::run()
{
    _tick = _ticker->val();
}

}
