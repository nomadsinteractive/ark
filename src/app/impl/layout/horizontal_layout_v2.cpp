#include "app/impl/layout/horizontal_layout_v2.h"

#include "core/util/math.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"

namespace ark {

V2 HorizontalLayoutV2::inflate(const std::vector<sp<LayoutParam>>& children)
{
    float width = 0, height = 0;
    for(const sp<LayoutParam>& i : children)
    {
        width += i->offsetWidth();
        height = std::max(i->offsetHeight(), height);
    }
    return V2(width, height);
}

std::vector<V2> HorizontalLayoutV2::place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent)
{
    std::vector<V2> positions;
    float weights = 0;
    float spaceAvailable = parent.contentWidth();
    for(const sp<LayoutParam>& i : slots)
    {
        weights += i->weight();
        spaceAvailable -= i->offsetWidth();
    }

    float unitWeight = weights == 0 ? 0 : spaceAvailable / weights;
    Rect r(0, 0, parent.contentWidth(), parent.contentHeight());

    for(const sp<LayoutParam>& i : slots)
    {
        float width = i->weight() == 0 ? i->offsetWidth() : i->weight() * unitWeight;
        const V2 pos = LayoutUtil::place(i->gravity(), V2(width, i->offsetHeight()), r);
        positions.push_back(pos);
        if(Math::almostEqual(pos.x(), r.left()))
            r.setLeft(r.left() + width);
        else
            r.setRight(r.left());
    }

    return positions;
}

sp<LayoutV2> HorizontalLayoutV2::BUILDER::build(const Scope& /*args*/)
{
    return sp<HorizontalLayoutV2>::make();
}

}
