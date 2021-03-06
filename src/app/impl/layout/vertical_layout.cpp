#include "app/impl/layout/vertical_layout.h"

#include "core/base/api.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "app/impl/layout/gravity_layout.h"

namespace ark {

VerticalLayout::VerticalLayout(const LayoutParam::Gravity gravity)
    : _place_gravity(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_HORIZONTAL)),
      _end_gravity(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_VERTICAL))
{
}

void VerticalLayout::begin(Context& ctx, LayoutParam& /*layoutParam*/)
{
    float contentHeight = 0;
    for(const sp<LayoutParam>& i : ctx._layout_param_descriptor())
        contentHeight += i->offsetHeight();

    _content_available = ctx._client_height - contentHeight;
    _flowy = 0.0f;
    _max_width = 0.0f;
}

Rect VerticalLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    Rect rect = GravityLayout::place(_place_gravity, ctx._client_width, ctx._client_height, layoutParam.calcLayoutWidth(ctx._client_width), layoutParam.calcLayoutHeight(_content_available));
    rect.translate(0, _flowy);
    _flowy += rect.height();
    if(_max_width < rect.height())
        _max_width = rect.height();
    return rect;
}

Rect VerticalLayout::end(Context& ctx)
{
    return GravityLayout::place(_end_gravity, ctx._client_width, ctx._client_height, _max_width, _flowy);
}

VerticalLayout::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& doc)
    : _gravity(Documents::ensureAttribute<LayoutParam::Gravity>(doc, Constants::Attributes::GRAVITY))
{
}

sp<Layout> VerticalLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<VerticalLayout>::make(_gravity);
}

VerticalLayout::DICTIONARY::DICTIONARY(const String& value)
    : _gravity(Strings::parse<LayoutParam::Gravity>(value))
{
}

sp<Layout> VerticalLayout::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<VerticalLayout>::make(_gravity);
}

}
