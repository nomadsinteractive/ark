#include "app/impl/layout/gravity_layout.h"

#include "core/base/api.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "app/view/view.h"


namespace ark {

GravityLayout::GravityLayout(const LayoutParam::Gravity& gravity)
    : _gravity(gravity)
{
}

Rect GravityLayout::place(LayoutParam::Gravity gravity, float clientWidth, float clientHeight, float width, float height)
{
    float x = 0, y = 0;
    switch(gravity & LayoutParam::CENTER_HORIZONTAL)
    {
        case LayoutParam::LEFT:
        break;
        case LayoutParam::RIGHT:
            x = clientWidth - width;
        break;
        case LayoutParam::CENTER_HORIZONTAL:
            x = (clientWidth - width) / 2;
        break;
    }
    switch(gravity & LayoutParam::CENTER_VERTICAL)
    {
        case LayoutParam::TOP:
            y = 0;
        break;
        case LayoutParam::BOTTOM:
            y = clientHeight - height;
        break;
        case LayoutParam::CENTER_VERTICAL:
            y = (clientHeight - height) / 2;
        break;
    }
    return Rect(x, y, x + width, y + height);
}

float GravityLayout::place(LayoutParam::Gravity gravity, float size, float available)
{
    switch(gravity)
    {
        case LayoutParam::LEFT:
        case LayoutParam::TOP:
            return 0;
        case LayoutParam::RIGHT:
        case LayoutParam::BOTTOM:
            return available - size;
        case LayoutParam::CENTER_HORIZONTAL:
        case LayoutParam::CENTER_VERTICAL:
            return (available - size) / 2;
        default:
        break;
    }
    return 0;
}

void GravityLayout::begin(Context& /*ctx*/, LayoutParam& /*layoutParam*/)
{
}

Rect GravityLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    LayoutParam::Gravity gravity = layoutParam.gravity() == LayoutParam::NONE ? _gravity : layoutParam.gravity();
    return GravityLayout::place(gravity, ctx._client_width, ctx._client_height, layoutParam.calcLayoutWidth(ctx._client_width), layoutParam.calcLayoutHeight(ctx._client_height));
}

Rect GravityLayout::end(Context& ctx)
{
    return Rect(0.0f, 0.0f, ctx._client_width, ctx._client_height);
}

GravityLayout::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest)
    : _gravity(Documents::getAttribute<LayoutParam::Gravity>(manifest, Constants::Attributes::GRAVITY, LayoutParam::Gravity::LEFT))
{
}

sp<Layout> GravityLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::adopt(new GravityLayout(_gravity));
}

}
