#include "app/impl/layout/horizontal_layout_v2.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"

namespace ark {

V2 HorizontalLayoutV2::inflate(const std::vector<sp<LayoutV2::Slot>>& children)
{
    float width = 0, height = 0;
    for(const sp<LayoutV2::Slot>& i : children)
    {
        width += i->_size.x();
        height = std::max(i->_size.y(), height);
    }
    return V2(width, height);
}

std::vector<V2> HorizontalLayoutV2::place(const std::vector<sp<LayoutV2::Slot>>& children, const sp<Slot>& parent)
{
    std::vector<V2> slots;
    float weights = 0;
    float spaceAvailable = parent->_size.x();
    for(const sp<LayoutV2::Slot>& i : children)
    {
        weights += i->_weight;
        spaceAvailable -= i->_size.x();
    }

    float unitWeight = weights == 0 ? 0 : spaceAvailable / weights;
    Rect r(0, 0, parent->_size.x(), parent->_size.y());

    for(const sp<LayoutV2::Slot>& i : children)
    {
        float width = i->_weight == 0 ? i->_size.x() : i->_weight * unitWeight;
        spaceAvailable -= i->_size.x();
        slots.push_back(LayoutUtil::place(i->_gravity, V2(width, i->_size.y()), r));
        r.setLeft(r.left() + width);
    }

    return slots;
}

sp<LayoutV2> HorizontalLayoutV2::BUILDER::build(const Scope& /*args*/)
{
    return sp<HorizontalLayoutV2>::make();
}

}
