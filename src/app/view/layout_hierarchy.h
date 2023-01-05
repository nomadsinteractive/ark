#ifndef ARK_APP_VIEW_LAYOUT_HIERARCHY_H_
#define ARK_APP_VIEW_LAYOUT_HIERARCHY_H_

#include <vector>

#include "core/forwarding.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/inf/layout_event_listener.h"

namespace ark {

class LayoutHierarchy : public Holder {
private:
    class Slot : public Holder {
    public:
        Slot(const sp<Renderer>& renderer, bool layoutRequested);

        virtual void traverse(const Visitor& visitor) override;

        void updateLayoutPosition(const V2& position, float clientHeight);

        bool isDisposed() const;
        bool isVisible() const;
        bool layoutRequested() const;

        void updateLayout();
        void wrapContentLayout() const;

        [[deprecated]]
        void render(RenderRequest& renderRequest, const V3& position);

        bool onEventDispatch(const Event& event, float x, float y);

        sp<LayoutParam> getLayoutParam() const;

        sp<LayoutV3::Node> makeLayoutNode() const;

    private:
        V2 _position;
        bool _layout_requested;
        [[deprecated]]
        sp<Renderer> _renderer;
        sp<View> _view;
        sp<ViewGroup> _view_group;
        sp<LayoutEventListener> _layout_event_listener;
        sp<Disposed> _disposed;
        sp<Visibility> _visible;
        friend class LayoutHierarchy;

    };

public:
    LayoutHierarchy(sp<Layout> layout, sp<LayoutV3> layoutV3);

    virtual void traverse(const Visitor& visitor) override;

    [[deprecated]]
    void render(RenderRequest& renderRequest, const V3& position) const;
    bool onEvent(const Event& event, float x, float y) const;

    void updateLayout(const sp<LayoutParam>& layoutParam);

    void addRenderer(const sp<Renderer>& renderer);

    sp<LayoutV3::Node> makeLayoutNode(sp<LayoutParam> layoutParam) const;
    void doLayoutInflat(sp<LayoutParam> layoutParam);

private:
    bool isLayoutNeeded(const LayoutParam& layoutParam, bool& inflateNeeded);

    std::pair<std::vector<sp<Slot>>, std::vector<sp<LayoutParam>>> getLayoutItems() const;

private:
    sp<Layout> _layout;
    sp<LayoutV3> _layout_v3;
    sp<LayoutV3::Node> _layout_node;
    V3 _layout_size;

    std::vector<sp<Slot>> _slots;
    std::vector<sp<Slot>> _incremental;

    friend class ViewGroup;
};

}

#endif
