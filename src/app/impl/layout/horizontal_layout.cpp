#include "app/impl/layout/horizontal_layout.h"

#include "core/util/documents.h"
#include "core/types/null.h"

#include "graphics/base/size.h"

#include "app/impl/layout/gravity_layout.h"
#include "app/view/view.h"


namespace ark {

HorizontalLayout::HorizontalLayout(const View::Gravity& gravity)
    : _place_gravity(static_cast<View::Gravity>(gravity & View::CENTER_VERTICAL)),
      _end_gravity(static_cast<View::Gravity>(gravity & View::CENTER_HORIZONTAL))
{
}

void HorizontalLayout::begin(Context& ctx, LayoutParam& /*layoutParam*/)
{
    for(const sp<LayoutParam>& i : ctx._layout_param_descriptor())
        ctx._content_width += i->offsetWidth();
    _content_available = ctx._client_width - ctx._content_width;
    _flowx = 0.0f;
    _max_height = 0.0f;
}

Rect HorizontalLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    Rect rect = GravityLayout::place(_place_gravity, ctx._client_width, ctx._client_height, layoutParam.calcLayoutWidth(_content_available), layoutParam.calcLayoutHeight(ctx._client_width));
    rect.translate(_flowx, 0);
    _flowx += rect.width();
    if(_max_height < rect.height())
        _max_height = rect.height();
    return rect;
}

Rect HorizontalLayout::end(Context& ctx)
{
    return GravityLayout::place(_end_gravity, ctx._client_width, ctx._client_height, _flowx, _max_height);
}

HorizontalLayout::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& doc)
    : _gravity(Documents::getAttribute<View::Gravity>(doc, Constants::Attributes::GRAVITY, View::Gravity::TOP))
{
}

sp<Layout> HorizontalLayout::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<Layout>::adopt(new HorizontalLayout(_gravity));
}

}
