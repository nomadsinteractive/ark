#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/render_group.h"

#include "app/forwarding.h"
#include "app/inf/event_listener.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API Activity final : public EventListener, public Renderer, public Renderer::Group {
public:
//  [[script::bindings::auto]]
    Activity();
    Activity(sp<RenderGroup> renderGroup);
    ~Activity() override;

//  [[script::bindings::auto]]
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;
//  [[script::bindings::auto]]
    void addRenderLayer(sp<Renderer> renderLayer, sp<Boolean> discarded = nullptr);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;
    bool onEvent(const Event& event) override;

//  [[script::bindings::auto]]
    void addEventListener(sp<EventListener> eventListener, const Traits& traits);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Activity> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Activity> build(const Scope& args) override;

    private:
        builder<View> _root_view;
        builder<RenderGroup> _render_group;
    };

private:
    sp<RenderGroup> _render_group;
    sp<ResourceLoader> _resource_loader;
    op<EventListenerList> _event_listeners;
};

}
