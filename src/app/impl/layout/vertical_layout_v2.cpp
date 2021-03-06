#include "app/impl/layout/vertical_layout_v2.h"

#include "core/util/math.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"

namespace ark {

VerticalLayoutV2::VerticalLayoutV2(LayoutParam::Gravity layoutGravity)
    : _layout_gravity(layoutGravity)
{
}

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

std::vector<V2> VerticalLayoutV2::place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize)
{
    std::vector<V2> positions;
    float weights = 0;
    float spaceAvailable = parent.contentHeight();
    for(const sp<LayoutParam>& i : slots)
    {
        weights += i->weight();
        spaceAvailable -= i->offsetHeight();
    }

    bool hasWeight = weights != 0.0f;
    float unitWeight = hasWeight ? spaceAvailable / weights : 0;
    const V2 layoutPos = LayoutUtil::place(_layout_gravity, contentSize, Rect(0, 0, parent.contentWidth(), parent.contentHeight()));
    Rect r = hasWeight ? Rect(0, 0, parent.contentWidth(), parent.contentWidth()) : Rect(layoutPos.x(), layoutPos.y(), contentSize.x(), contentSize.y());

    LayoutParam::FlexFlow ff = LayoutParam::FLEX_FLOW_ROW;
    for(const sp<LayoutParam>& i : slots)
    {
        float width = i->calcLayoutWidth(parent.contentWidth());
        float height = i->calcLayoutHeight(i->hasWeight() ? i->weight() * unitWeight : parent.contentHeight());
        positions.push_back(LayoutUtil::place(i->gravity(), ff, V2(width, height), r));
    }

    return positions;
}

VerticalLayoutV2::BUILDER::BUILDER(const String& gravity)
    : _layout_gravity(Strings::parse<LayoutParam::Gravity>(gravity))
{
}

sp<LayoutV2> VerticalLayoutV2::BUILDER::build(const Scope& /*args*/)
{
    return sp<VerticalLayoutV2>::make(_layout_gravity);
}

}
