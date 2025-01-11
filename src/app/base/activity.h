#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/render_group.h"

#include "core/base/resource_loader.h"
#include "app/forwarding.h"
#include "app/inf/event_listener.h"

namespace ark {

class ARK_API Activity final : public EventListener, public Renderer, public Renderer::Group {
public:
    Activity(sp<View> view, sp<RenderGroup> renderGroup, sp<ResourceLoader> resourceLoader);
    ~Activity() override;

//  [[script::bindings::auto]]
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void render(RenderRequest& renderRequest, const V3& position) override;
    bool onEvent(const Event& event) override;

    sp<Entity> makeEntity(Traits components = Traits()) const;

//  [[script::bindings::auto]]
    sp<Arena> makeArena() const;

//  [[script::bindings::auto]]
    Box getReference(const String& id) const;

//  [[script::bindings::property]]
    const sp<ResourceLoader>& resourceLoader() const;

//  [[script::bindings::property]]
    sp<BoxBundle> refs() const;
    [[deprecated]]
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
    void addRenderLayer(sp<Renderer> renderLayer, sp<Boolean> discarded = nullptr);

//  [[script::bindings::property]]
    void setView(sp<View> view);
//  [[script::bindings::property]]
    const sp<View>& view() const;

//  [[script::bindings::auto]]
    void addView(sp<View> view, sp<Boolean> discarded = nullptr);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Activity> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Activity> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        SafeBuilder<ResourceLoader> _resource_loader;
        builder<View> _root_view;
    };

private:
    sp<View> _view;
    sp<RenderGroup> _render_group;
    sp<ResourceLoader> _resource_loader;
    op<EventListenerList> _event_listeners;
};

}
