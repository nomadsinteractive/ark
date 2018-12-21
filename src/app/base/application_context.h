#ifndef ARK_APP_BASE_APPLICATION_CONTEXT_H_
#define ARK_APP_BASE_APPLICATION_CONTEXT_H_

#include <map>

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/forwarding.h"
#include "core/inf/message_loop.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationContext : public MessageLoop {
public:
    ApplicationContext(const sp<ApplicationResource>& applicationResource, const sp<RenderEngine>& renderEngine);
    ~ApplicationContext() override;

    sp<ResourceLoader> createResourceLoader(const String& name, const sp<Scope>& args);
    sp<ResourceLoader> createResourceLoader(const String& name, const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Scope>& args);

    const sp<ApplicationResource>& applicationResource() const;
    const sp<RenderEngine>& renderEngine() const;
    const sp<ResourceManager>& resourceManager() const;
    const sp<RenderController>& renderController() const;
    const sp<ResourceLoader>& resourceLoader() const;
    const sp<Executor>& executor() const;

    const List<String>& argv() const;
    const sp<Clock>& clock() const;

    bool onEvent(const Event& event);

    void addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& expired = nullptr);
    void addEventListener(const sp<EventListener>& eventListener);
    void setDefaultEventListener(const sp<EventListener>& eventListener);

    virtual void post(const sp<Runnable>& task, float delay = 0.0f) override;
    virtual void schedule(const sp<Runnable>& task, float interval) override;

    virtual uint64_t pollOnce() override;

    void addStringBundle(const String& name, const sp<StringBundle>& stringBundle);
    sp<String> getString(const String& resid);

    sp<Runnable> defer(const sp<Runnable>& task) const;

    const Color& backgroundColor() const;
    void setBackgroundColor(const Color& backgroundColor);

    void pause();
    void resume();

    template<typename T> void deferUnref(sp<T>&& inst) {
        const sp<T> s = inst;
        _render_controller->deferUnref(s.pack());
    }

private:
    void initMessageLoop();
    void initResourceLoader(const document& manifest);
    sp<ResourceLoader> createResourceLoader(const sp<Dictionary<document>>& documentDictionary, const sp<ResourceLoaderContext>& resourceLoaderContext);

    void dispose();
    void waitForFinish();

private:
    List<String> _argv;

    sp<ApplicationResource> _application_resource;
    sp<RenderEngine> _render_engine;
    sp<RenderController> _render_controller;
    sp<Clock> _clock;
    sp<MessageLoopThread> _message_loop_application;
    sp<Executor> _executor;

    op<EventListenerList> _event_listeners;
    sp<EventListener> _default_event_listener;

    sp<ResourceLoader> _resource_loader;
    sp<StringTable> _string_table;

    Color _background_color;

    friend class Ark;
    friend class Application;

};

}

#endif
