#include "app/impl/layout/frame_layout.h"

#include "app/view/layout_param.h"
#include "app/util/layout_util.h"

namespace ark {

bool FrameLayout::update(uint64_t timestamp)
{
    return false;
}

void FrameLayout::inflate(sp<Node> rootNode)
{
}

sp<LayoutV3> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<LayoutV3>::make<FrameLayout>();
}

}
