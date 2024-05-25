#include "app/impl/layout/vertical_layout.h"

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
    const V2 layoutOffsetPos = rootNode.offsetPosition();
    LayoutParam::JustifyContent justifyContent = rootNode._layout_param->justifyContent();
    switch(justifyContent) {
        case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
        case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
        case LayoutParam::JUSTIFY_CONTENT_CENTER: {
            float y = justifyContent == LayoutParam::JUSTIFY_CONTENT_FLEX_START ? 0 : rootNode.contentHeight() - calcChildNodesTotalHeight();
            if(justifyContent == LayoutParam::JUSTIFY_CONTENT_CENTER)
                y /= 2;

            for(const ViewHierarchy::Slot& i : rootNode._view_hierarchy->updateSlots())
            {
                const sp<LayoutV3::Node>& layoutNode = i.layoutNode();
                float x = calcItemOffsetPosition(rootNode, layoutNode);
                layoutNode->setOffsetPosition(layoutOffsetPos + V2(x, y));
                layoutNode->setPaddings(i.layoutParam()->paddings().val());
                layoutNode->setMargins(i.layoutParam()->margins().val());
                y += layoutNode->occupyHeight();
            }
        }
        case LayoutParam::JUSTIFY_CONTENT_SPACE_BETWEEN: {

        }
        case LayoutParam::JUSTIFY_CONTENT_SPACE_AROUND: {

        }
        case LayoutParam::JUSTIFY_CONTENT_SPACE_EVENLY: {

        }
    }
    return true;
}

float VerticalLayout::calcChildNodesTotalHeight() const
{
    float totalHeight = 0;
    for(const ViewHierarchy::Slot& i : _root_node->_view_hierarchy->updateSlots())
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
