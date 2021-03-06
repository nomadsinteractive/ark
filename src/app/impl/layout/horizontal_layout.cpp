#include "app/impl/layout/horizontal_layout.h"

#include "core/util/documents.h"
#include "core/types/null.h"

#include "graphics/base/size.h"

#include "app/impl/layout/gravity_layout.h"
#include "app/util/layout_util.h"
#include "app/view/view.h"


namespace ark {

HorizontalLayout::HorizontalLayout(const LayoutParam::Gravity& gravity)
    : _place_gravity(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_VERTICAL)),
      _end_gravity(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_HORIZONTAL))
{
}

void HorizontalLayout::begin(Context& ctx, LayoutParam& /*layoutParam*/)
{
    float contentWidth = 0;
    for(const sp<LayoutParam>& i : ctx._layout_param_descriptor())
        contentWidth += i->offsetWidth();
    _content_available = ctx._client_width - contentWidth;
    _flowx = 0.0f;
    _max_height = 0.0f;
}

Rect HorizontalLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    Rect rect = GravityLayout::place(_place_gravity, ctx._client_width, ctx._client_height, layoutParam.calcLayoutWidth(_content_available), layoutParam.calcLayoutHeight(ctx._client_height));
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
    : _gravity(Documents::getAttribute<LayoutParam::Gravity>(doc, Constants::Attributes::GRAVITY, LayoutParam::Gravity::GRAVITY_TOP))
{
}

sp<Layout> HorizontalLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::adopt(new HorizontalLayout(_gravity));
}

HorizontalLayout::DICTIONARY::DICTIONARY(const String& value)
    : _gravity(Strings::parse<LayoutParam::Gravity>(value))
{
}

sp<Layout> HorizontalLayout::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<HorizontalLayout>::make(_gravity);
}

}
