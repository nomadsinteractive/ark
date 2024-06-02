#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/owned_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_phrase.h"

#include "core/base/resource_loader.h"
#include "app/forwarding.h"
#include "app/inf/event_listener.h"

namespace ark {

class ARK_API Arena final : public EventListener, public Renderer, public Renderer::Group {
public:
    Arena(sp<View> view, sp<ResourceLoader> resourceLoader);
    ~Arena() override;

//  [[script::bindings::auto]]
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void render(RenderRequest& renderRequest, const V3& position) override;
    bool onEvent(const Event& event) override;

    sp<Entity> makeEntity(Traits components = Traits()) const;

//  [[script::bindings::loader]]
    template<typename T> const sp<T> load(const String& name, const Scope& args) {
        DCHECK(_resource_loader, "Trying to load objects on a discarded Arena");
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
    void addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    void pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    void addLayer(sp<Renderer> layer, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    void addRenderLayer(sp<Renderer> renderLayer, sp<Boolean> discarded = nullptr);

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
    RendererPhrase _renderer_phrase;
};

}
