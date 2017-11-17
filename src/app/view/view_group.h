#ifndef ARK_APP_VIEW_VIEW_GROUP_H_
#define ARK_APP_VIEW_VIEW_GROUP_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/expirable_item_list.h"
#include "core/inf/builder.h"
#include "core/types/class.h"

#include "graphics/inf/renderer.h"
#include "graphics/base/frame.h"

#include "app/inf/event_listener.h"
#include "app/view/view.h"

namespace ark {

class ARK_API ViewGroup : public View, public Renderer, public Renderer::Group, Implements<ViewGroup, View, Block, Renderer, Renderer::Group> {
private:
    class Placement {
    public:
        Placement(const sp<Renderer>& renderer);

        const sp<Renderer>& renderer() const;
        const sp<ViewGroup>& viewGroup() const;
        const sp<View>& view() const;

        bool isExpired() const;

        void doPlace(float clientHeight, const sp<Layout>& layout);
        void doEnd(const Rect& p);

        void render(RenderRequest& renderRequest, float x, float y) const;

        bool onEventDispatch(const Event& event, float x, float y);

    private:
        float _x, _y;
        sp<Renderer> _renderer;
        sp<View> _view;
        sp<ViewGroup> _view_group;
        sp<RendererDelegate> _renderer_delegate;
        sp<Expired> _expirable;
    };

public:
    ViewGroup(const Frame& background, const sp<Layout>& layout, const sp<LayoutParam>& layoutParam);
    ~ViewGroup();

    virtual void addRenderer(const sp<Renderer>& renderer) override;
    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    bool onEvent(const Event& event, float x, float y);

//  [[plugin::builder("view-group")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BeanFactory _parent;
        document _manifest;

        sp<Builder<Layout>> _layout;
        sp<Builder<Renderer>> _background;
        sp<Builder<LayoutParam>> _layout_param;
    };

private:
    void doLayout();

private:
    sp<Renderer> _background;
    sp<Layout> _layout;

    ExpirableItemList<Renderer> _renderers;
    List<sp<Placement>> _placments;

    bool _layout_requested;
};

}

#endif
