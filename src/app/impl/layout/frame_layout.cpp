#include "app/impl/layout/frame_layout.h"

#include "app/traits/layout_param.h"
#include "app/util/layout_util.h"

namespace ark {

bool FrameLayout::update(uint64_t timestamp)
{
    return false;
}

void FrameLayout::inflate(sp<Node> rootNode)
{
}

sp<Layout> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::make<FrameLayout>();
}

}
