#include "app/impl/layout/horizontal_layout.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark {

HorizontalLayout::HorizontalLayout(LayoutParam::Align alignItems)
    : _align_items(alignItems)
{
}

bool HorizontalLayout::update(uint64_t timestamp)
{
    DCHECK(_root_node->_view_hierarchy, "");

    const Node& rootNode = *_root_node;
    const std::vector<sp<View>>& childNodes = rootNode._view_hierarchy->updateSlotsAndLayoutNodes();

    float totalWidth = 0;
    for(const View& i : childNodes)
        totalWidth += i.layoutNode()->occupyWidth();

    const V2 layoutOffsetPos = rootNode.offsetPosition();
    LayoutParam::JustifyContent justifyContent = rootNode._layout_param->justifyContent();
    const auto [start, space] = LayoutUtil::calcFlowDirection(justifyContent, rootNode.contentWidth(), totalWidth, childNodes.size());
    float x = start;
    for(const View& i : childNodes)
    {
        Node& layoutNode = i.layoutNode();
        float y = LayoutUtil::calcItemOffsetY(_align_items, rootNode, layoutNode);
        layoutNode.setOffsetPosition(layoutOffsetPos + V2(x, y));
        x += (layoutNode.occupyWidth() + space);
    }
    return true;
}

void HorizontalLayout::inflate(sp<Node> rootNode)
{
    _root_node = std::move(rootNode);
}

sp<Layout> HorizontalLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<HorizontalLayout>::make(_align_items);
}

HorizontalLayout::BUILDER::BUILDER(const String& alignItems)
    : _align_items(Strings::eval<LayoutParam::Align>(alignItems))
{
}

}
