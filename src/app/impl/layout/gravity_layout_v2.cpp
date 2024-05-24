#include "app/impl/layout/gravity_layout_v2.h"

#include "graphics/base/rect.h"
#include "graphics/base/size.h"

#include "app/util/layout_util.h"

namespace ark {

GravityLayoutV2::GravityLayoutV2(const LayoutParam::Gravity& gravity)
    : _gravity(gravity)
{
}

V2 GravityLayoutV2::inflate(const std::vector<sp<LayoutParam>>& slots)
{
    return LayoutUtil::inflate(slots);
}

std::vector<V2> GravityLayoutV2::place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& /*contentSize*/)
{
    std::vector<V2> positions;
    const Rect bounds(0, 0, parent.contentWidth(), parent.contentWidth());
    for(const sp<LayoutParam>& i : slots)
    {
        float width = i->calcLayoutWidth(parent.contentWidth());
        float height = i->calcLayoutHeight(parent.contentHeight());
        positions.push_back(LayoutUtil::place(i->gravity() == LayoutParam::GRAVITY_DEFAULT ? _gravity : i->gravity(), V2(width, height), bounds));
    }

    return positions;
}

GravityLayoutV2::STYLE::STYLE(const String& gravity)
    : _gravity(Strings::eval<LayoutParam::Gravity>(gravity)) {
}

sp<Layout> GravityLayoutV2::STYLE::build(const Scope& /*args*/)
{
    return sp<Layout>::make<GravityLayoutV2>(_gravity);
}

}
