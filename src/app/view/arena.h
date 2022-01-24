#ifndef ARK_APP_VIEW_STAGE_H_
#define ARK_APP_VIEW_STAGE_H_

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/types/owned_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/base/resource_loader.h"
#include "app/forwarding.h"
#include "app/inf/event_listener.h"
#include "app/view/view_group.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Arena final : public EventListener, public Renderer, public Renderer::Group, public Holder {
public:
    Arena(const sp<ViewGroup>& view, const sp<ResourceLoader>& resourceLoader);
    ~Arena() override;

//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

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
    void addEventListener(const sp<EventListener>& eventListener, int32_t priority = 0);
//  [[script::bindings::auto]]
    void addLayer(sp<Renderer> layer);
//  [[script::bindings::auto]]
    void addRenderLayer(sp<Renderer> renderLayer);

//  [[script::bindings::property]]
    void setView(const sp<Renderer>& view);
//  [[script::bindings::property]]
    const sp<ViewGroup>& view() const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Arena> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Arena> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<ResourceLoader>> _resource_loader;
        SafePtr<Builder<Layout>> _layout;
        sp<Builder<LayoutParam>> _layout_param;
        String _background;
        String _view;
    };

private:
    op<EventListenerList> _event_listeners;
    sp<ViewGroup> _view_group;
    sp<ResourceLoader> _resource_loader;
    DisposableItemList<Renderer> _layers;
    DisposableItemList<Renderer> _render_layers;
};

}

#endif
