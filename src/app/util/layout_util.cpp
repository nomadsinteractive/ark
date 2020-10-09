#include "app/util/layout_util.h"

#include "graphics/base/rect.h"

namespace ark {

V2 LayoutUtil::place(LayoutParam::Gravity gravity, const V2& clientSize, const Rect& parent)
{
    float x = 0, y = 0;
    switch(gravity & LayoutParam::GRAVITY_CENTER_HORIZONTAL)
    {
        case LayoutParam::GRAVITY_LEFT:
        break;
        case LayoutParam::GRAVITY_RIGHT:
            x = clientSize.x() - parent.width();
        break;
        case LayoutParam::GRAVITY_CENTER_HORIZONTAL:
            x = (clientSize.x() - parent.width()) / 2;
        break;
    }
    switch(gravity & LayoutParam::GRAVITY_CENTER_VERTICAL)
    {
        case LayoutParam::GRAVITY_TOP:
            y = 0;
        break;
        case LayoutParam::GRAVITY_BOTTOM:
            y = clientSize.y() - parent.height();
        break;
        case LayoutParam::GRAVITY_CENTER_VERTICAL:
            y = (clientSize.y() - parent.height()) / 2;
        break;
    }
    return V2(x + parent.left(), y + parent.top());
}

float LayoutUtil::place(LayoutParam::Gravity gravity, float size, float available)
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
