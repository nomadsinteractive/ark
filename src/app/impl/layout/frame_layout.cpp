#include "app/impl/layout/frame_layout.h"

#include "app/view/layout_param.h"

namespace ark {

void FrameLayout::begin(LayoutParam& layoutParam)
{
}

Rect FrameLayout::place(LayoutParam& layoutParam)
{
    return Rect(0, 0, layoutParam.contentWidth(), layoutParam.contentHeight());
}

Rect FrameLayout::end()
{
    return Rect(0, 0, 0, 0);
}

FrameLayout::BUILDER::BUILDER()
{
}

sp<Layout> FrameLayout::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<Layout>::adopt(new FrameLayout());
}

}
