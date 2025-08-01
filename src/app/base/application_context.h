#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/message_loop.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/executor/executor_worker_thread.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"

#include "app/forwarding.h"
#include "app/impl/event_listener/event_listener_list.h"
#include "app/inf/event_listener.h"

namespace ark {

class ARK_API ApplicationContext {
public:
    ApplicationContext(sp<ApplicationBundle> applicationBundle, sp<RenderEngine> renderEngine);

    sp<ResourceLoader> createResourceLoader(const String& name, const Scope& args);
    sp<ResourceLoader> createResourceLoader(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    const sp<ApplicationBundle>& applicationBundle() const;
    const sp<ApplicationFacade>& applicationFacade() const;
    const sp<ApplicationEventListener>& applicationEventListener() const;
    void setApplicationEventListener(sp<ApplicationEventListener> applicationEventListener);

    const sp<RenderEngine>& renderEngine() const;
    const sp<RenderController>& renderController() const;
    const sp<ResourceLoader>& resourceLoader() const;
    const sp<Interpreter>& interpreter() const;

    const sp<Executor>& executorMain() const;
    const sp<Executor>& executorThreadPool() const;

    const Vector<String>& argv() const;

    const sp<Clock>& sysClock() const;
    const sp<Clock>& appClock() const;

    void pushAppClock(sp<Numeric> timeScale = nullptr);
    sp<Clock> popAppClock();

    const sp<Numeric::Impl>& appClockInterval() const;
    uint64_t timestamp() const;
    uint64_t onRenderTick();

    const sp<Vec2Impl>& cursorPosition() const;

    bool onEvent(const Event& event);

    void addPreComposeRunnable(sp<Runnable> runnable, sp<Boolean> cancelled);

    void addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);
    void pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);

    sp<MessageLoop> makeMessageLoop(const sp<Clock>& clock);
    const sp<MessageLoop>& messageLoopApp() const;
    const sp<MessageLoop>& messageLoopCore() const;
    const sp<MessageLoop>& messageLoopRenderer() const;

    void runAtCoreThread(sp<Runnable> task) const;
    void runAtCoreThread(std::function<void()> task) const;

    void addStringBundle(const String& name, const sp<StringBundle>& stringBundle);
    Optional<String> getString(const String& resid, bool alert);
    Vector<String> getStringArray(const String& resid);

    sp<Runnable> defer(const sp<Runnable>& task) const;

    const Color& backgroundColor() const;
    void setBackgroundColor(const Color& backgroundColor);

    void pause();
    void resume();

    void updateRenderState();

    bool isPaused() const;

    template<typename T> void deferUnref(sp<T> inst) const {
        _render_controller->deferUnref(Box(std::move(inst)));
    }

private:
    void initialize(const document& manifest);
    void finalize() const;

    sp<ResourceLoader> createResourceLoaderImpl(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);
    document createResourceLoaderManifest(const document& manifest) const;

    struct AppClock {
        AppClock(sp<Numeric> timeScale);

        sp<Variable<uint64_t>> _steady;
        SafeVar<Numeric> _time_scale;

        sp<Variable<uint64_t>::Impl> _tick;
        sp<Numeric::Impl> _interval;

        sp<Clock> _clock;

        uint64_t _timestamp;

        uint64_t onTick();
    };

    class ExecutorWorkerStrategy;

private:
    Vector<String> _argv;
    sp<Variable<uint64_t>> _ticker;
    sp<Vec2Impl> _cursor_position;
    sp<Vec2Impl> _cursor_frag_coord;

    sp<ApplicationBundle> _application_bundle;
    sp<ApplicationFacade> _application_facade;
    sp<ApplicationEventListener> _application_event_listener;

    sp<RenderEngine> _render_engine;
    sp<RenderController> _render_controller;
    sp<Clock> _sys_clock;
    Vector<AppClock> _app_clocks;

    sp<ExecutorWorkerStrategy> _worker_strategy;

    sp<Executor> _executor_main;
    sp<Executor> _executor_thread_pool;

    sp<MessageLoop> _message_loop_renderer;
    sp<MessageLoop> _message_loop_core;

    Vector<sp<MessageLoop>> _app_message_loops;

    sp<ResourceLoader> _resource_loader;
    sp<StringTable> _string_table;

    sp<Interpreter> _interpreter;

    EventListenerList _event_listeners;
    Color _background_color;
    bool _paused;

    friend class Ark;
    friend class Application;
    friend class ApplicationDelegate;
};

}
