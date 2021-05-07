#include "app/impl/layout/frame_layout_v2.h"

#include "app/view/layout_param.h"
#include "app/util/layout_util.h"

namespace ark {

V2 FrameLayoutV2::inflate(const std::vector<sp<LayoutParam>>& slots)
{
    return LayoutUtil::inflate(slots);
}

std::vector<V2> FrameLayoutV2::place(const std::vector<sp<LayoutParam>>& children, const LayoutParam& /*parent*/, const V2& /*contentSize*/)
{
    std::vector<V2> slots(children.size());
    return slots;
}

sp<LayoutV2> FrameLayoutV2::BUILDER::build(const Scope& /*args*/)
{
    return sp<LayoutV2>::make<FrameLayoutV2>();
}

}
