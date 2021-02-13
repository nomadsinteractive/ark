#include "app/util/layout_util.h"

#include "graphics/base/rect.h"

namespace ark {

V2 LayoutUtil::place(LayoutParam::Gravity gravity, const V2& clientSize, const Rect& parent)
{
    float x = placeOne(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_HORIZONTAL), clientSize.x(), parent.width());
    float y = placeOne(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_VERTICAL), clientSize.y(), parent.height());
    return V2(x + parent.left(), y + parent.top());
}

float LayoutUtil::placeOne(LayoutParam::Gravity gravity, float size, float available)
{
    switch(gravity)
    {
        case LayoutParam::GRAVITY_LEFT:
        case LayoutParam::GRAVITY_TOP:
            return 0;
        case LayoutParam::GRAVITY_RIGHT:
        case LayoutParam::GRAVITY_BOTTOM:
            return available - size;
        case LayoutParam::GRAVITY_CENTER_HORIZONTAL:
        case LayoutParam::GRAVITY_CENTER_VERTICAL:
            return (available - size) / 2;
        default:
        break;
    }
    return 0;
}

}
