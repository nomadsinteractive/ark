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

sp<Layout> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::adopt(new FrameLayout());
}

sp<Layout> FrameLayout::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<FrameLayout>::make();
}

}
