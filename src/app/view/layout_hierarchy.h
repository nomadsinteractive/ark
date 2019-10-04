#ifndef ARK_APP_VIEW_CALCULATED_LAYOUT_H_
#define ARK_APP_VIEW_CALCULATED_LAYOUT_H_

#include <vector>

#include "core/forwarding.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"

namespace ark {

class LayoutHierarchy : public Holder {
private:
    class Slot : public Holder {
    public:
        Slot(const sp<Renderer>& renderer, bool layoutRequested);

        virtual void traverse(const Visitor& visitor) override;

        bool isDisposed() const;
        bool layoutRequested() const;

        void updateLayout();

        void doPlace(Layout::Context& ctx, float clientHeight, const sp<Layout>& layout);
        void doWrapContentPlace(Layout::Context& ctx, const sp<Layout>& layout, Rect& contentRect) const;
        void doLayoutEnd(const Rect& p);

        void render(RenderRequest& renderRequest, const V3& position);

        bool onEventDispatch(const Event& event, float x, float y);

        sp<LayoutParam> getLayoutParam() const;

    private:
        float _x, _y;
        float _layout_width;
        float _layout_height;

        bool _layout_requested;
        sp<Renderer> _renderer;
        sp<View> _view;
        sp<ViewGroup> _view_group;
        sp<Disposed> _disposed;
        sp<Visibility> _visibility;

        friend class LayoutHierarchy;

    };

public:
    LayoutHierarchy(const sp<Layout>& layout);

    virtual void traverse(const Visitor& visitor) override;

    void render(RenderRequest& renderRequest, const V3& position) const;
    bool onEvent(const Event& event, float x, float y) const;

    void updateLayout(LayoutParam& layoutParam);

    void doWrapContentLayout(Layout::Context& ctx, LayoutParam& layoutParam);

    void addRenderer(const sp<Renderer>& renderer);

private:
    bool isLayoutNeeded(const LayoutParam& layoutParam);
    std::vector<sp<LayoutParam>> getLayoutParams() const;

private:
    sp<Layout> _layout;
    V3 _layout_size;

    std::vector<sp<Slot>> _slots;
    std::vector<sp<Slot>> _incremental;

};

}

#endif
