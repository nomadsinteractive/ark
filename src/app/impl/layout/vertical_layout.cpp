#include "app/impl/layout/vertical_layout.h"

#include "app/view/view.h"

namespace ark {

VerticalLayout::VerticalLayout(LayoutParam::Align alignItems)
    : _align_items(alignItems)
{
}

std::pair<float, float> calcFlowDirection(LayoutParam::JustifyContent justifyContent, float totalSpace, float childrenSpace, size_t childCount)
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

bool VerticalLayout::update(uint64_t timestamp)
{
    DCHECK(_root_node->_view_hierarchy, "");

    const Node& rootNode = *_root_node;
    const std::vector<sp<ViewHierarchy::Slot>>& childNodes = rootNode._view_hierarchy->updateSlots();

    for(const ViewHierarchy::Slot& i : childNodes)
    {
        const sp<LayoutV3::Node>& layoutNode = i.layoutNode();
        layoutNode->setPaddings(i.layoutParam()->paddings().val());
        layoutNode->setMargins(i.layoutParam()->margins().val());
    }

    const V2 layoutOffsetPos = rootNode.offsetPosition();
    LayoutParam::JustifyContent justifyContent = rootNode._layout_param->justifyContent();
    const auto [start, space] = calcFlowDirection(justifyContent, rootNode.contentHeight(), calcChildNodesTotalHeight(childNodes), childNodes.size());
    float y = start;
    for(const ViewHierarchy::Slot& i : childNodes)
    {
        const sp<LayoutV3::Node>& layoutNode = i.layoutNode();
        float x = calcItemOffsetPosition(rootNode, layoutNode);
        layoutNode->setOffsetPosition(layoutOffsetPos + V2(x, y));
        y += (layoutNode->occupyHeight() + space);
    }
    return true;
}

float VerticalLayout::calcChildNodesTotalHeight(const std::vector<sp<ViewHierarchy::Slot>>& childNodes)
{
    float totalHeight = 0;
    for(const ViewHierarchy::Slot& i : childNodes)
        totalHeight += i.layoutNode()->occupyHeight();
    return totalHeight;
}

float VerticalLayout::calcItemOffsetPosition(const Node& rootNode, Node& item) const
{
    float offset = 0;
    switch (_align_items) {
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

void VerticalLayout::inflate(sp<Node> rootNode)
{
    _root_node = std::move(rootNode);
}

sp<LayoutV3> VerticalLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<VerticalLayout>::make(_align_items);
}

VerticalLayout::BUILDER::BUILDER(const String& alignItems)
    : _align_items(Strings::eval<LayoutParam::Align>(alignItems))
{
}

}
