#pragma once

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/types/owned_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "core/base/resource_loader.h"
#include "app/forwarding.h"
#include "app/inf/event_listener.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Arena final : public EventListener, public Renderer, public Renderer::Group, public Holder {
public:
    Arena(sp<View> view, sp<ResourceLoader> resourceLoader);
    ~Arena() override;

//  [[script::bindings::auto]]
    virtual void addRenderer(const sp<Renderer>& renderer) override;

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
    virtual bool onEvent(const Event& event) override;

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::loader]]
    template<typename T> const sp<T> load(const String& name, const Scope& args) {
        DCHECK(_resource_loader, "Trying to load objects on a disposed Arena");
        const sp<T> bean = _resource_loader->load<T>(name, args);
        DCHECK(bean, "Cannot build object \"%s\"", name.c_str());
        return bean;
    }

//  [[script::bindings::auto]]
    sp<Renderer> loadRenderer(const String& name, const Scope& args);
//  [[script::bindings::auto]]
    Box getReference(const String& id) const;

//  [[script::bindings::property]]
    const sp<ResourceLoader>& resourceLoader() const;

//  [[script::bindings::property]]
    sp<BoxBundle> refs() const;
//  [[script::bindings::property]]
    sp<BoxBundle> layers() const;
//  [[script::bindings::property]]
    sp<BoxBundle> renderLayers() const;
//  [[script::bindings::property]]
    sp<BoxBundle> packages() const;

//  [[script::bindings::auto]]
    void addEventListener(sp<EventListener> eventListener, sp<Boolean> disposed = nullptr);
//  [[script::bindings::auto]]
    void pushEventListener(sp<EventListener> eventListener, sp<Boolean> disposed = nullptr);
//  [[script::bindings::auto]]
    void addLayer(sp<Renderer> layer);
//  [[script::bindings::auto]]
    void addRenderLayer(sp<Renderer> renderLayer);

//  [[script::bindings::property]]
    void setView(sp<View> view);
//  [[script::bindings::property]]
    const sp<View>& view() const;

//  [[script::bindings::auto]]
    void addView(sp<View> view, sp<Boolean> disposable = nullptr);

    void dispose();

//  [[plugin::builder]]
    class BUILDER : public Builder<Arena> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Arena> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<ResourceLoader>> _resource_loader;
        SafePtr<Builder<View>> _view;
    };

private:
    sp<View> _view;
    sp<ResourceLoader> _resource_loader;
    op<EventListenerList> _event_listeners;

    DList<sp<Renderer>> _renderers;
    DList<sp<Renderer>> _layers;
    DList<sp<Renderer>> _render_layers;
};

}
