#include "app/base/application_context.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/message_loop.h"
#include "core/base/plugin_manager.h"
#include "core/base/resource_loader.h"
#include "core/base/string_table.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/executor/executor_worker_thread.h"
#include "core/impl/executor/executor_thread_pool.h"
#include "core/impl/runnable/runnable_by_function.h"
#include "core/inf/interpreter.h"
#include "core/inf/runnable.h"
#include "core/types/global.h"

#include "graphics/impl/vec/vec2_impl.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/renderer_factory.h"

#include "app/base/application_bundle.h"
#include "app/base/application_manifest.h"
#include "app/base/event.h"
#include "app/impl/event_listener/event_listener_list.h"

#include "platform/platform.h"


namespace ark {

namespace {

class DeferedRunnable final : public Runnable {
public:
    DeferedRunnable(const sp<MessageLoop>& messageLoop, sp<Runnable> runnable)
        : _message_loop(messageLoop), _runnable(std::move(runnable)) {
    }

    void run() override {
        _message_loop->post(_runnable, 0);
    }

private:
    sp<MessageLoop> _message_loop;
    sp<Runnable> _runnable;
};

class NoneInterpreter final : public Interpreter {
public:
    void initialize() override {}
    void execute(const sp<Asset>& source) override {}

    Box call(const Box& func, const Arguments& args) override
    {
        return nullptr;
    }
    Box attr(const Box& obj, StringView name) override
    {
        return nullptr;
    }
};

}

class ApplicationContext::Ticker final : public Variable<uint64_t> {
public:
    Ticker()
        : _steady_clock(Platform::getSteadyClock()), _val(_steady_clock->val()) {
    }

    bool update(uint64_t timestamp) override
    {
        _val = _steady_clock->val();
        return true;
    }

    uint64_t val() override
    {
        return _val;
    }

private:
    sp<Variable<uint64_t>> _steady_clock;
    std::atomic<uint64_t> _val;
};

class ApplicationContext::ExecutorWorkerStrategy final : public ExecutorWorkerThread::Strategy {
public:
    ExecutorWorkerStrategy(const sp<AppClock>& appClock, sp<MessageLoop> messageLoop)
        : _app_clock(appClock), _message_loop(std::move(messageLoop)) {
    }

    void onStart() override
    {
    }

    void onExit() override
    {
    }

    void onBusy() override
    {
        _message_loop->pollOnce();
        for(const sp<MessageLoop>& i : _app_message_loops)
            i->pollOnce();
    }

    void onIdle(Thread& thread) override
    {
        onBusy();
    }

    void onException(const std::exception& e) override
    {
        throw e;
    }

    sp<AppClock> _app_clock;
    sp<MessageLoop> _message_loop;
    U_FList<sp<MessageLoop>> _app_message_loops;
};

ApplicationContext::AppClock::AppClock()
    : _steady(Platform::getSteadyClock()), _tick(sp<Variable<uint64_t>::Impl>::make(0)), _interval(sp<Numeric::Impl>::make(0)), _clock(sp<Clock>::make(_tick)), _next_timestamp(_steady->val()), _timestamp(_next_timestamp)
{
}

uint64_t ApplicationContext::AppClock::onTick()
{
    _next_timestamp = _steady->val();
    const uint64_t interval = std::min<uint64_t>(_next_timestamp - _timestamp, 1000000 / 24);
    _interval->set(interval / 1000000.0f);
    _tick->set(_tick->val() + interval);

    const uint64_t timestamp = _timestamp;
    _timestamp = _next_timestamp;
    return timestamp;
}

ApplicationContext::ApplicationContext(sp<ApplicationBundle> applicationBundle, sp<RenderEngine> renderEngine)
    : _ticker(sp<Ticker>::make()), _cursor_position(sp<Vec2Impl>::make()), _cursor_frag_coord(sp<Vec2Impl>::make()), _application_bundle(std::move(applicationBundle)), _render_engine(std::move(renderEngine)),
      _render_controller(sp<RenderController>::make(_render_engine, _application_bundle->bitmapBundle(), _application_bundle->bitmapBoundsBundle())), _sys_clock(sp<Clock>::make(_ticker)), _app_clock(sp<AppClock>::make()),
      _worker_strategy(sp<ExecutorWorkerStrategy>::make(_app_clock, sp<MessageLoop>::make(_ticker))), _executor_main(sp<Executor>::make<ExecutorWorkerThread>(_worker_strategy, "Executor")),
      _executor_thread_pool(sp<Executor>::make<ExecutorThreadPool>(_executor_main)), _string_table(Global<StringTable>()), _background_color(0, 0, 0), _paused(false)
{
    const Ark& ark = Ark::instance();

    for(int32_t i = 0; i < ark.argc(); i++)
        _argv.emplace_back(ark.argv()[i]);
}

void ApplicationContext::initialize(const document& manifest)
{
    const Ark& ark = Ark::instance();
    const document doc = createResourceLoaderManifest(manifest);
    _resource_loader = createResourceLoaderImpl(manifest, nullptr);
    _resource_loader->import(doc, _resource_loader->beanFactory());

    _message_loop_renderer = sp<MessageLoop>::make(_ticker);
    _message_loop_core = ark.manifest()->application()._message_loop == ApplicationManifest::MESSAGE_LOOP_TYPE_RENDER ? _message_loop_renderer : _worker_strategy->_message_loop;
    _message_loop_app = makeMessageLoop(_app_clock->_clock);

    if(const document& interpreter = ark.manifest()->interpreter())
        _interpreter = _resource_loader->beanFactory().build<Interpreter>(interpreter, {});
    else
        _interpreter = sp<Interpreter>::make<NoneInterpreter>();
}

void ApplicationContext::finalize() const
{
    _executor_main.cast<ExecutorWorkerThread>()->terminate();
    _executor_thread_pool.cast<ExecutorThreadPool>()->releaseAll(true);
    _executor_main.cast<ExecutorWorkerThread>()->tryJoin();
}

sp<ResourceLoader> ApplicationContext::createResourceLoader(const String& name, const Scope& args)
{
    if(const Identifier id(Identifier::parse(name)); id.isVal())
    {
        const document doc = _application_bundle->loadDocument(name);
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
    const sp<DictionaryByAttributeName> documentDictionary = sp<DictionaryByAttributeName>::make(doc, constants::ID);
    const sp<BeanFactory> beanFactory = Ark::instance().createBeanFactory(documentDictionary);
    const sp<ResourceLoaderContext> context = resourceLoaderContext ? resourceLoaderContext : sp<ResourceLoaderContext>::make(_application_bundle->documents(), _application_bundle->bitmapBundle(), _application_bundle->bitmapBoundsBundle(), _render_controller);

    for(const sp<Plugin>& i : Global<PluginManager>()->plugins())
        i->loadResourceLoader(beanFactory, context);

    return sp<ResourceLoader>::make(beanFactory);
}

document ApplicationContext::createResourceLoaderManifest(const document& manifest) const
{
    DASSERT(manifest);
    const String src = Documents::getAttribute(manifest, constants::SRC);
    const document doc = src ? _application_bundle->loadDocument(src) : manifest;
    DCHECK_WARN(doc == manifest || manifest->children().size() == 0, "\"%s\" has already specified a ResourceLoader src \"%s\", its content will be ignored", Documents::toString(manifest).c_str(), src.c_str());
    return doc;
}

const sp<ApplicationBundle>& ApplicationContext::applicationBundle() const
{
    return _application_bundle;
}

const sp<ApplicationFacade>& ApplicationContext::applicationFacade() const
{
    return _application_facade;
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

const sp<Interpreter>& ApplicationContext::interpreter() const
{
    return _interpreter;
}

const sp<Executor>& ApplicationContext::executorMain() const
{
    return _executor_main;
}

const sp<Executor>& ApplicationContext::executorThreadPool() const
{
    return _executor_thread_pool;
}

const Vector<String>& ApplicationContext::argv() const
{
    return _argv;
}

const sp<Clock>& ApplicationContext::sysClock() const
{
    return _sys_clock;
}

const sp<Clock>& ApplicationContext::appClock() const
{
    return _app_clock->_clock;
}

const sp<Numeric::Impl>& ApplicationContext::appClockInterval() const
{
    return _app_clock->_interval;
}

const sp<Vec2Impl>& ApplicationContext::cursorPosition() const
{
    return _cursor_position;
}

uint64_t ApplicationContext::timestamp() const
{
    return _app_clock->_timestamp;
}

bool ApplicationContext::onEvent(const Event& event)
{
    if(event.action() == Event::ACTION_UP || event.action() == Event::ACTION_DOWN || event.action() == Event::ACTION_MOVE)
    {
        _cursor_position->set(event.xy());
        _cursor_frag_coord->set(event.xyFragCoord());
    }
    return _event_listeners.onEvent(event) || (_default_event_listener && _default_event_listener->onEvent(event));
}

void ApplicationContext::addPreComposeRunnable(sp<Runnable> runnable, sp<Boolean> cancelled)
{
    _render_controller->addPreComposeRunnable(std::move(runnable), std::move(cancelled));
}

void ApplicationContext::addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _event_listeners.addEventListener(std::move(eventListener), std::move(discarded));
}

void ApplicationContext::pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    _event_listeners.pushEventListener(std::move(eventListener), std::move(discarded));
}

void ApplicationContext::setDefaultEventListener(sp<EventListener> eventListener)
{
    _default_event_listener = std::move(eventListener);
}

sp<MessageLoop> ApplicationContext::makeMessageLoop(const sp<Clock>& clock)
{
    sp<MessageLoop> messageLoop = sp<MessageLoop>::make(clock->ticker());
    _worker_strategy->_app_message_loops.emplace_back(messageLoop);
    return messageLoop;
}

const sp<MessageLoop>& ApplicationContext::messageLoopApp() const
{
    return _message_loop_app;
}

const sp<MessageLoop>& ApplicationContext::messageLoopCore() const
{
    return _message_loop_core;
}

const sp<MessageLoop>& ApplicationContext::messageLoopRenderer() const
{
    return _message_loop_renderer;
}

void ApplicationContext::runAtCoreThread(sp<Runnable> task)
{
    _message_loop_core->post(std::move(task), 0);
}

void ApplicationContext::runAtCoreThread(std::function<void()> task)
{
    _message_loop_core->post(sp<RunnableByFunction>::make(std::move(task)), 0);
}

void ApplicationContext::addStringBundle(const String& name, const sp<StringBundle>& stringBundle)
{
    _string_table->addStringBundle(name, stringBundle);
}

Optional<String> ApplicationContext::getString(const String& resid, const bool alert)
{
    ASSERT(resid);
    const Identifier id = resid.at(0) == '@' ? Identifier::parse(resid, Identifier::ID_TYPE_AUTO, false) : Identifier::parseRef(resid, false);
    return _string_table->getString(id.package(), id.ref(), alert);
}

Vector<String> ApplicationContext::getStringArray(const String& resid)
{
    ASSERT(resid);
    const Identifier id = resid.at(0) == '@' ? Identifier::parse(resid, Identifier::ID_TYPE_AUTO, false) : Identifier::parseRef(resid, false);
    return _string_table->getStringArray(id.package(), id.ref(), true);
}

sp<Runnable> ApplicationContext::defer(const sp<Runnable>& task) const
{
    return sp<Runnable>::make<DeferedRunnable>(_message_loop_core, task);
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
    _sys_clock->pause();
}

void ApplicationContext::resume()
{
    _paused = false;
    _sys_clock->resume();
}

void ApplicationContext::updateRenderState()
{
    _ticker->update(0);
    _message_loop_renderer->pollOnce();
}

bool ApplicationContext::isPaused() const
{
    return _paused;
}

}
