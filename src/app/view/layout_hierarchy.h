#ifndef ARK_APP_VIEW_CALCULATED_LAYOUT_H_
#define ARK_APP_VIEW_CALCULATED_LAYOUT_H_

#include <vector>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class LayoutHierarchy {
private:
    class Slot {
    public:
        Slot(const sp<Renderer>& renderer, bool layoutRequested);

        bool isExpired() const;
        bool layoutRequested() const;

        void updateLayout();

        void doPlace(float clientHeight, const sp<Layout>& layout);
        void doWrapContentPlace(const sp<Layout>& layout, Rect& contentRect) const;
        void doLayoutEnd(const Rect& p);

        void render(RenderRequest& renderRequest, float x, float y);

        bool onEventDispatch(const Event& event, float x, float y);

    private:
        float _x, _y;
        float _layout_width;
        float _layout_height;

        bool _layout_requested;
        sp<Renderer> _renderer;
        sp<View> _view;
        sp<ViewGroup> _view_group;
        sp<RendererDelegate> _renderer_delegate;
        sp<Lifecycle> _expirable;

    };
public:
    LayoutHierarchy(const sp<Layout>& layout);

    void render(RenderRequest& renderRequest, float x, float y) const;
    bool onEvent(const Event& event, float x, float y) const;

    bool isLayoutNeeded();

    void doLayout(LayoutParam& layoutParam);
    void doWrapContentLayout(LayoutParam& layoutParam);

    void addRenderer(const sp<Renderer>& renderer);


private:
    sp<Layout> _layout;

    std::vector<sp<Slot>> _slots;
};

}

#endif
