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
        width = std::max(width, i.occupyWidth());
        height = std::max(height, i.occupyHeight());
    }
    return V2(width, height);
}

std::pair<float, float> LayoutUtil::calcFlowDirection(LayoutParam::JustifyContent justifyContent, float totalSpace, float childrenSpace, size_t childCount)
{
    switch(justifyContent) {
        case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
            return {0.0f, 0.0f};
        case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
            return {totalSpace - childrenSpace, 0.0f};
        case LayoutParam::JUSTIFY_CONTENT_CENTER:
            return {(totalSpace - childrenSpace) / 2, 0.0f};
        case LayoutParam::JUSTIFY_CONTENT_SPACE_BETWEEN:
            return {0.0f, (totalSpace - childrenSpace) / (childCount - 1)};
        case LayoutParam::JUSTIFY_CONTENT_SPACE_AROUND: {
            float space = (totalSpace - childrenSpace) / childCount;
            return {space / 2, space};
        }
        case LayoutParam::JUSTIFY_CONTENT_SPACE_EVENLY: {
            float space = (totalSpace - childrenSpace) / (childCount + 1);
            return {space, space};
        }
    }
    return {0.0f, 0.0f};
}

float LayoutUtil::calcItemOffsetX(LayoutParam::Align align, const Layout::Node& rootNode, Layout::Node& item)
{
    float offset = 0;
    switch (align) {
        case LayoutParam::ALIGN_CENTER:
            offset = (rootNode.contentWidth() - item.occupyWidth()) / 2;
            break;
        case LayoutParam::ALIGN_FLEX_END:
            offset = rootNode.contentWidth() - item.occupyWidth();
            break;
        case LayoutParam::ALIGN_STRETCH:
            item.setSize(V2(rootNode.size()->x(), item.size()->y()));
        case LayoutParam::ALIGN_AUTO:
        case LayoutParam::ALIGN_FLEX_START:
        case LayoutParam::ALIGN_BASELINE:
        case LayoutParam::ALIGN_SPACE_BETWEEN:
        case LayoutParam::ALIGN_SPACE_AROUND:
        default:
            break;
    }
    return offset;
}

float LayoutUtil::calcItemOffsetY(LayoutParam::Align align, const Layout::Node& rootNode, Layout::Node& item)
{
    float offset = 0;
    switch (align) {
        case LayoutParam::ALIGN_CENTER:
            offset = (rootNode.contentHeight() - item.occupyHeight()) / 2;
            break;
        case LayoutParam::ALIGN_FLEX_END:
            offset = rootNode.contentHeight() - item.occupyHeight();
            break;
        case LayoutParam::ALIGN_STRETCH:
            item.setSize(V2(rootNode.size()->x(), item.size()->y()));
        case LayoutParam::ALIGN_AUTO:
        case LayoutParam::ALIGN_FLEX_START:
        case LayoutParam::ALIGN_BASELINE:
        case LayoutParam::ALIGN_SPACE_BETWEEN:
        case LayoutParam::ALIGN_SPACE_AROUND:
        default:
            break;
    }
    return offset;
}

}
