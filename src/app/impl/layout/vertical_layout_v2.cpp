#include "app/impl/layout/vertical_layout_v2.h"

#include "core/util/math.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"

namespace ark {

V2 VerticalLayoutV2::inflate(const std::vector<sp<LayoutParam>>& children)
{
    float width = 0, height = 0;
    for(const sp<LayoutParam>& i : children)
    {
        width = std::max(i->offsetWidth(), width);
        height += i->offsetHeight();
    }
    return V2(width, height);
}

std::vector<V2> VerticalLayoutV2::place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent)
{
    std::vector<V2> positions;
    float weights = 0;
    float spaceAvailable = parent.contentHeight();
    for(const sp<LayoutParam>& i : slots)
    {
        weights += i->weight();
        spaceAvailable -= i->offsetHeight();
    }

    float unitWeight = weights == 0 ? 0 : spaceAvailable / weights;
    Rect r(0, 0, parent.contentWidth(), parent.contentHeight());

    for(const sp<LayoutParam>& i : slots)
    {
        float height = i->weight() == 0 ? i->offsetHeight() : i->weight() * unitWeight;
        const V2 pos = LayoutUtil::place(i->gravity(), V2(i->offsetWidth(), height), r);
        positions.push_back(pos);
        if(Math::almostEqual(pos.x(), r.left()))
            r.setTop(r.top() + height);
        else
            r.setBottom(r.top());
    }

    return positions;
}

sp<LayoutV2> VerticalLayoutV2::BUILDER::build(const Scope& /*args*/)
{
    return sp<VerticalLayoutV2>::make();
}

}
