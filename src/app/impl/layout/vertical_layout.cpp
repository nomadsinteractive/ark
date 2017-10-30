#include "app/impl/layout/vertical_layout.h"

#include "core/base/api.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "app/impl/layout/gravity_layout.h"

namespace ark {

VerticalLayout::VerticalLayout(const View::Gravity gravity)
    : _place_gravity(static_cast<View::Gravity>(gravity & View::CENTER_HORIZONTAL)),
      _end_gravity(static_cast<View::Gravity>(gravity & View::CENTER_VERTICAL))
{
}

void VerticalLayout::begin(LayoutParam& layoutParam)
{
    _content_width = layoutParam.contentWidth();
    _content_height = layoutParam.contentHeight();
    _flowy = 0.0f;
    _max_width = 0.0f;
}

Rect VerticalLayout::place(LayoutParam& layoutParam)
{
    Rect rect = GravityLayout::place(_place_gravity, _content_width, _content_height, layoutParam.calcLayoutWidth(_content_width), layoutParam.calcLayoutHeight(_content_height - _flowy));
    rect.translate(0, _flowy);
    _flowy += rect.height();
    if(_max_width < rect.height())
        _max_width = rect.height();
    return rect;
}

Rect VerticalLayout::end()
{
    return GravityLayout::place(_end_gravity, _content_width, _content_height, _max_width, _flowy);
}

VerticalLayout::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& doc)
    : _gravity(Documents::ensureAttribute<View::Gravity>(doc, Constants::Attributes::GRAVITY))
{
}

sp<Layout> VerticalLayout::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<VerticalLayout>::make(_gravity);
}

}
