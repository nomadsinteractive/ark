#include "app/impl/layout/vertical_layout.h"

#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark {

VerticalLayout::VerticalLayout(LayoutParam::FlexDirection flexDirection)
    : _flex_direction(flexDirection)
{
}

bool VerticalLayout::update(uint64_t timestamp)
{
    DCHECK(_root_node->_view_hierarchy, "");

    const V2 layoutOffsetPos = _root_node->offsetPosition();
    LayoutParam::JustifyContent justifyContent = _root_node->_layout_param->justifyContent();
    switch(justifyContent) {
        case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
        case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
        case LayoutParam::JUSTIFY_CONTENT_CENTER: {
            float y = justifyContent == LayoutParam::JUSTIFY_CONTENT_FLEX_START ? 0 : _root_node->contentHeight() - calcChildNodesTotalHeight();
            if(justifyContent == LayoutParam::JUSTIFY_CONTENT_CENTER)
                y /= 2;

            for(const ViewHierarchy::Slot& i : _root_node->_view_hierarchy->updateSlots())
            {
                const sp<LayoutV3::Node>& layoutNode = i.layoutNode();
                layoutNode->setOffsetPosition(layoutOffsetPos + V2(0, y));
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

void VerticalLayout::inflate(sp<Node> rootNode)
{
    _root_node = std::move(rootNode);
}

sp<LayoutV3> VerticalLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<VerticalLayout>::make(_flex_direction);
}

VerticalLayout::BUILDER::BUILDER(const String& flexDirection)
    : _flex_direction(Strings::eval<LayoutParam::FlexDirection>(flexDirection))
{
}

}
