#ifndef ARK_APP_VIEW_VIEW_HIERARCHY_H_
#define ARK_APP_VIEW_VIEW_HIERARCHY_H_

#include <vector>

#include "core/forwarding.h"
#include "core/inf/holder.h"
#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/inf/layout_event_listener.h"

namespace ark {

class ARK_API ViewHierarchy : public Holder, public Updatable {
public:
    class ARK_API Slot : public Holder {
    public:
        Slot(const sp<Renderer>& renderer, sp<View> view, bool layoutRequested);

        virtual void traverse(const Visitor& visitor) override;

        const sp<View>& view() const;

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

        sp<LayoutV3::Node> layoutNode() const;

    private:
        bool _layout_requested;
        [[deprecated]]
        sp<Renderer> _renderer;
        sp<View> _view;
        sp<LayoutEventListener> _layout_event_listener;
        sp<Disposed> _disposed;
        sp<Visibility> _visible;

        friend class ViewHierarchy;
    };

public:
    ViewHierarchy(sp<Layout> layout, sp<LayoutV3> layoutV3);

    virtual void traverse(const Visitor& visitor) override;

    virtual bool update(uint64_t timestamp) override;

    const sp<LayoutV3>& layout() const;

    [[deprecated]]
    void render(RenderRequest& renderRequest, const V3& position);
    bool onEvent(const Event& event, float x, float y) const;

    void updateLayout(View& view, uint64_t timestamp, bool isDirty);
    const std::vector<sp<Slot>>& updateSlots();

    void addRenderer(const sp<Renderer>& renderer);
    void addView(sp<View> view);

private:
    bool isLayoutNeeded(const LayoutParam& layoutParam, bool& inflateNeeded);

    std::pair<std::vector<sp<Slot>>, std::vector<sp<LayoutParam>>> getLayoutItems() const;

private:
    sp<Layout> _layout;
    sp<LayoutV3> _layout_v3;
    V3 _layout_size;

    std::vector<sp<Slot>> _slots;
    std::vector<sp<Slot>> _incremental;
};

}

#endif
