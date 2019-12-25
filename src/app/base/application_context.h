#ifndef ARK_APP_BASE_APPLICATION_CONTEXT_H_
#define ARK_APP_BASE_APPLICATION_CONTEXT_H_

#include <map>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/message_loop.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationContext {
public:
    ApplicationContext(const sp<ApplicationResource>& applicationResource, const sp<RenderEngine>& renderEngine);
    ~ApplicationContext();

    sp<ResourceLoader> createResourceLoader(const String& name, const Scope& args);
    sp<ResourceLoader> createResourceLoader(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    const sp<ApplicationResource>& applicationResource() const;
    const sp<RenderEngine>& renderEngine() const;
    const sp<RenderController>& renderController() const;
    const sp<ResourceLoader>& resourceLoader() const;
    const sp<Executor>& executor() const;

    const std::vector<String>& argv() const;

    const sp<Clock>& clock() const;

    bool onEvent(const Event& event);

    void addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& disposed = nullptr);
    void addEventListener(const sp<EventListener>& eventListener);
    void setDefaultEventListener(const sp<EventListener>& eventListener);

    void post(const sp<Runnable>& task, float delay = 0);
    void schedule(const sp<Runnable>& task, float interval);

    void post(std::function<void()> task, float delay = 0);
    void schedule(std::function<bool()> task, float interval);

    void postToRenderer(std::function<void()> task);

    void addStringBundle(const String& name, const sp<StringBundle>& stringBundle);
    sp<String> getString(const String& resid);
    std::vector<String> getStringArray(const String& resid);

    sp<Runnable> defer(const sp<Runnable>& task) const;

    const Color& backgroundColor() const;
    void setBackgroundColor(const Color& backgroundColor);

    void pause();
    void resume();

    void updateRenderState();

    bool isPaused() const;

    template<typename T> void deferUnref(sp<T>&& inst) const {
        _render_controller->deferUnref(inst);
    }

    template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate, const sp<Boolean>& disposed = nullptr) const {
        return _render_controller->synchronize<T>(delegate, disposed);
    }

private:
    void initResourceLoader(const document& manifest);

    sp<ResourceLoader> createResourceLoaderImpl(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);
    document createResourceLoaderManifest(const document& manifest);

    sp<MessageLoop> makeMessageLoop();

    class Ticker : public Variable<uint64_t> {
    public:
        Ticker();

        virtual bool update(uint64_t timestamp) override;
        virtual uint64_t val() override;

    private:
        sp<Variable<uint64_t>> _steady_clock;
        std::atomic<uint64_t> _val;
    };

private:
    std::vector<String> _argv;
    sp<Ticker> _ticker;

    sp<ApplicationResource> _application_resource;
    sp<RenderEngine> _render_engine;
    sp<RenderController> _render_controller;
    sp<Clock> _clock;
    sp<MessageLoopDefault> _render_message_loop;
    sp<MessageLoop> _message_loop;
    sp<Executor> _executor;

    op<EventListenerList> _event_listeners;
    sp<EventListener> _default_event_listener;

    sp<ResourceLoader> _resource_loader;
    sp<StringTable> _string_table;

    Color _background_color;
    bool _paused;

    friend class Ark;
    friend class Application;

};

}

#endif
