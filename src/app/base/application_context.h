#ifndef ARK_APP_BASE_APPLICATION_CONTEXT_H_
#define ARK_APP_BASE_APPLICATION_CONTEXT_H_

#include <map>

#include "core/base/api.h"
#include "core/collection/expirable_item_list.h"
#include "core/collection/list.h"
#include "core/forwarding.h"
#include "core/inf/message_loop.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationContext : public MessageLoop {
public:
    ApplicationContext(const sp<ApplicationResource>& applicationResource);
    ~ApplicationContext();

//  [[script::bindings::auto]]
    sp<ResourceLoader> createResourceLoader(const String& name);
    sp<ResourceLoader> createResourceLoader(const String& name, const sp<ResourceLoaderContext>& resourceLoaderContext);

    const sp<ApplicationResource>& applicationResource() const;
    const sp<GLResourceManager>& glResourceManager() const;
    const sp<RenderController>& renderController() const;

    const sp<Executor>& executor() const;

//  [[script::bindings::property]]
    const List<String>& argv() const;
//  [[script::bindings::property]]
    const sp<Clock>& clock() const;

    bool onEvent(const Event& event);

//  [[script::bindings::auto]]
    void addPreRenderTask(const sp<Runnable>& task, const sp<Boolean>& expired = nullptr);
//  [[script::bindings::auto]]
    void addEventListener(const sp<EventListener>& eventListener);
//  [[script::bindings::auto]]
    void setDefaultEventListener(const sp<EventListener>& eventListener);

//  [[script::bindings::auto]]
    virtual void post(const sp<Runnable>& task, float delay = 0.0f) override;
//  [[script::bindings::auto]]
    virtual void schedule(const sp<Runnable>& task, float interval) override;

    virtual uint64_t pollOnce() override;

//  [[script::bindings::auto]]
    void addStringBundle(const String& name, const sp<StringBundle>& stringBundle);
//  [[script::bindings::auto]]
    sp<String> getString(const String& resid);

//  [[script::bindings::auto]]
    sp<Runnable> defer(const sp<Runnable>& task) const;

    void pause();
    void resume();

    template<typename T> void deferUnref(const sp<T>& inst) {
        _render_controller->deferUnref(inst.pack());
    }

private:
    class EngineTicker;

private:
    void initMessageLoop();
    void initResourceLoader(const document& manifest);
    sp<ResourceLoader> createResourceLoader(const sp<Dictionary<document>>& documentDictionary, const sp<ResourceLoaderContext>& resourceLoaderContext);

    void dispose();
    void waitForFinish();

private:
    List<String> _argv;

    sp<ApplicationResource> _application_resource;
    sp<RenderViewFactory> _render_view_factory;
    sp<EngineTicker> _ticker;
    sp<Clock> _clock;
    sp<MessageLoopThread> _message_loop_application;
    sp<Executor> _executor;
    sp<RenderController> _render_controller;

    op<EventListenerList> _event_listeners;
    sp<EventListener> _default_event_listener;

    sp<ResourceLoader> _resource_loader;
    sp<StringTable> _string_table;

    friend class Ark;
    friend class Application;

};

}

#endif
