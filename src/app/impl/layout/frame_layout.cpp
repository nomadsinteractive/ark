#include "app/impl/layout/frame_layout.h"

#include "core/types/weak_ptr.h"
#include "core/util/numeric_type.h"
#include "core/util/variable_util.h"

#include "app/view/layout_param.h"
#include "app/impl/layout/gravity_layout.h"

namespace ark {

void FrameLayout::begin(Context& /*ctx*/, LayoutParam& /*layoutParam*/)
{
}

Rect FrameLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    return Rect(0, 0, layoutParam.calcLayoutWidth(ctx._client_width), layoutParam.calcLayoutHeight(ctx._client_height));
}

Rect FrameLayout::end(Context& /*ctx*/)
{
    return Rect(0, 0, 0, 0);
}

FrameLayout::BUILDER::BUILDER()
{
}

sp<Layout> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::adopt(new FrameLayout());
}

std::vector<V2> FrameLayoutNew::place(const std::vector<sp<LayoutV2::Slot>>& children, const sp<Slot>& /*parent*/)
{
    std::vector<V2> slots(children.size());
    return slots;
}

}
