#include "app/impl/layout/vertical_layout.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark {

VerticalLayout::VerticalLayout(LayoutParam::Align alignItems)
    : _align_items(alignItems)
{
}

bool VerticalLayout::update(uint64_t timestamp)
{
    DCHECK(_root_node->_view_hierarchy, "");

    const Node& rootNode = *_root_node;
    const std::vector<sp<View>>& childNodes = rootNode._view_hierarchy->updateSlotsAndLayoutNodes();

    float totalHeight = 0;
    for(const View& i : childNodes)
        totalHeight += i.layoutNode()->occupyHeight();

    const V2 layoutOffsetPos = rootNode.offsetPosition();
    LayoutParam::JustifyContent justifyContent = rootNode._layout_param->justifyContent();
    const auto [start, space] = LayoutUtil::calcFlowDirection(justifyContent, rootNode.contentHeight(), totalHeight, childNodes.size());
    float y = start;
    for(const View& i : childNodes)
    {
        Node& layoutNode = i.layoutNode();
        float x = LayoutUtil::calcItemOffsetX(_align_items, rootNode, layoutNode);
        layoutNode.setOffsetPosition(layoutOffsetPos + V2(x, y));
        y += (layoutNode.occupyHeight() + space);
    }
    return true;
}

void VerticalLayout::inflate(sp<Node> rootNode)
{
    _root_node = std::move(rootNode);
}

sp<Layout> VerticalLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<VerticalLayout>::make(_align_items);
}

VerticalLayout::BUILDER::BUILDER(const String& alignItems)
    : _align_items(Strings::eval<LayoutParam::Align>(alignItems))
{
}

}
