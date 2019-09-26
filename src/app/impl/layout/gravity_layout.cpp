#include "app/impl/layout/gravity_layout.h"

#include "core/base/api.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "app/view/view.h"


namespace ark {

GravityLayout::GravityLayout(const View::Gravity& gravity)
    : _gravity(gravity)
{
}

Rect GravityLayout::place(View::Gravity gravity, float clientWidth, float clientHeight, float width, float height)
{
    float x = 0, y = 0;
    switch(gravity & View::CENTER_HORIZONTAL)
    {
        case View::LEFT:
        break;
        case View::RIGHT:
            x = clientWidth - width;
        break;
        case View::CENTER_HORIZONTAL:
            x = (clientWidth - width) / 2;
        break;
    }
    switch(gravity & View::CENTER_VERTICAL)
    {
        case View::TOP:
            y = 0;
        break;
        case View::BOTTOM:
            y = clientHeight - height;
        break;
        case View::CENTER_VERTICAL:
            y = (clientHeight - height) / 2;
        break;
    }
    return Rect(x, y, x + width, y + height);
}

void GravityLayout::begin(Context& /*ctx*/, LayoutParam& /*layoutParam*/)
{
}

Rect GravityLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    return GravityLayout::place(_gravity, ctx._client_width, ctx._client_height, layoutParam.calcLayoutWidth(ctx._client_width), layoutParam.calcLayoutHeight(ctx._client_height));
}

Rect GravityLayout::end(Context& ctx)
{
    return Rect(0.0f, 0.0f, ctx._client_width, ctx._client_height);
}

GravityLayout::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& doc)
    : _gravity(Documents::getAttribute<View::Gravity>(doc, Constants::Attributes::GRAVITY, View::Gravity::LEFT))
{
}

sp<Layout> GravityLayout::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<Layout>::adopt(new GravityLayout(_gravity));
}

}
