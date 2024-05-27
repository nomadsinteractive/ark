#include "app/util/layout_util.h"

#include "graphics/base/rect.h"

namespace ark {

Rect LayoutUtil::flow(LayoutParam::FlexDirection flexDirection, const V2& size, Rect& available)
{
    switch(flexDirection) {
        case LayoutParam::FLEX_DIRECTION_COLUMN: {
            Rect allocated(available.left(), available.top(), available.left() + size.x(), available.bottom());
            available.setLeft(allocated.right());
            return allocated;
        }
        case LayoutParam::FLEX_DIRECTION_COLUMN_REVERSE: {
            Rect allocated(available.right() - size.x(), available.top(), available.right(), available.bottom());
            available.setRight(allocated.left());
            return allocated;
        }
        case LayoutParam::FLEX_DIRECTION_ROW: {
            Rect allocated(available.left(), available.top(), available.right(), available.top() + size.y());
            available.setTop(allocated.bottom());
            return allocated;
        }
        case LayoutParam::FLEX_DIRECTION_ROW_REVERSE: {
            Rect allocated(available.left(), available.bottom() - size.y(), available.right(), available.bottom());
            available.setBottom(allocated.top());
            return allocated;
        }
    }
    return available;
}

V2 LayoutUtil::inflate(const std::vector<sp<LayoutParam>>& slots)
{
    float width = 0, height = 0;
    for(const LayoutParam& i : slots)
    {
        width = std::max(width, i.offsetWidth());
        height = std::max(height, i.offsetHeight());
    }
    return V2(width, height);
}

V2 LayoutUtil::place(LayoutParam::Gravity gravity, const V2& size, const Rect& available)
{
    float x = placeOneDimension(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_HORIZONTAL), size.x(), available.width());
    float y = placeOneDimension(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_VERTICAL), size.y(), available.height());
    return V2(x + available.left(), y + available.top());
}

V2 LayoutUtil::place(LayoutParam::Gravity gravity, LayoutParam::FlexDirection flexDirection, const V2& size, Rect& available)
{
    const Rect allocated = flow(flexDirection, size, available);
    switch(flexDirection) {
        case LayoutParam::FLEX_DIRECTION_COLUMN:
        case LayoutParam::FLEX_DIRECTION_COLUMN_REVERSE:
            return place(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_VERTICAL), size, allocated);
        case LayoutParam::FLEX_DIRECTION_ROW:
        case LayoutParam::FLEX_DIRECTION_ROW_REVERSE:
            return place(static_cast<LayoutParam::Gravity>(gravity & LayoutParam::GRAVITY_CENTER_HORIZONTAL), size, allocated);
    }
    return place(gravity, size, allocated);
}

float LayoutUtil::placeOneDimension(LayoutParam::Gravity gravity, float size, float available)
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
