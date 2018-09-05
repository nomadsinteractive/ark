#ifndef ARK_APP_VIEW_STAGE_H_
#define ARK_APP_VIEW_STAGE_H_

#include "core/base/api.h"
#include "core/collection/expirable_item_list.h"
#include "core/inf/builder.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "app/base/resource_loader.h"
#include "app/forwarding.h"
#include "app/inf/event_listener.h"
#include "app/view/view_group.h"

namespace ark {

//[[script::bindings::container]]
class ARK_API Arena final : public Block, public EventListener, public Renderer, public Renderer::Delegate, public Renderer::Group {
public:
    Arena(const sp<Renderer>& rootView, const sp<ResourceLoader>& resourceLoader);
    ~Arena();

//  [[script::bindings::meta(absorb())]]
//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

//  [[script::bindings::property]]
    virtual const sp<Size>& size() override;

//  [[script::bindings::auto]]
    virtual void addRenderer(const sp<Renderer>& renderer) override;
//  [[script::bindings::auto]]
    virtual void setRendererDelegate(const sp<Renderer>& delegate) override;

    virtual void render(RenderRequest& renderRequest, float x, float y) override;
    virtual bool onEvent(const Event& event) override;

//  [[script::bindings::loader]]
    template<typename T> const sp<T> load(const String& name, const sp<Scope>& args = nullptr) {
        DCHECK(_resource_loader, "Trying to load objects on a disposed Arena");
        const sp<T> bean = _resource_loader->load<T>(name, args);
        DCHECK(bean, "Cannot build object \"%s\"", name.c_str());
        return bean;
    }

//  [[script::bindings::auto]]
    sp<Renderer> loadRenderer(const String& name, const sp<Scope>& args = nullptr);

//  [[script::bindings::property]]
    const sp<Scope>& refs() const;

//  [[script::bindings::auto]]
    void addEventListener(const sp<EventListener>& eventListener);
//  [[script::bindings::auto]]
    void addLayer(const sp<Renderer> &layer);

//  [[script::bindings::property]]
    void setView(const sp<Renderer>& view);
//  [[script::bindings::property]]
    const sp<ViewGroup>& view() const;

//  [[script::bindings::auto]]
    void dispose();

//  [[plugin::builder]]
    class BUILDER : public Builder<Arena> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Arena> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<Builder<ResourceLoader>> _resource_loader;
        sp<Builder<Layout>> _layout;
        sp<Builder<LayoutParam>> _layout_param;
        String _background;
        String _view;
    };

//  [[plugin::style("expired")]]
    class STYLE : public Builder<Arena> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Arena>>& delegate, const String& value);

        virtual sp<Arena> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Arena>> _delegate;
        sp<Builder<Lifecycle>> _expired;
    };

private:
    op<EventListenerList> _event_listeners;
    sp<Renderer> _renderer;
    sp<ViewGroup> _view_group;
    sp<ResourceLoader> _resource_loader;
    ExpirableItemList<Renderer> _layers;

};

}

#endif
