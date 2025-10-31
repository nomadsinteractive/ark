#include "app/impl/layout/vertical_layout.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark {

namespace {

class UpdatableVerticalLayout : public Updatable {
public:
    UpdatableVerticalLayout(Layout::Hierarchy hierarchy, LayoutParam::Align alignItems)
        : _hierarchy(std::move(hierarchy)), _align_items(alignItems) {
    }

    bool update(uint32_t tick) override {
        float totalHeight = 0;
        for(const auto& [node, child_nodes] : _hierarchy._child_nodes) {
            DCHECK_WARN(child_nodes.empty(), "VerticalLayout is not a recursively layout for now");
            node->update(tick);
            totalHeight += node->occupyHeight();
        }

        const Layout::Node& rootNode = *_hierarchy._node;
        const V2 layoutOffsetPos = rootNode.offsetPosition();
        LayoutParam::JustifyContent justifyContent = rootNode._layout_param->justifyContent();
        const auto [start, space] = LayoutUtil::calcFlowDirection(justifyContent, rootNode.contentHeight(), totalHeight, _hierarchy._child_nodes.size());
        float y = start;
        for(const auto& [node, child_nodes] : _hierarchy._child_nodes) {
            Layout::Node& layoutNode = *node;
            float x = LayoutUtil::calcItemOffsetX(_align_items, rootNode, layoutNode);
            layoutNode.setOffsetPosition(layoutOffsetPos + V2(x, y));
            y += (layoutNode.occupyHeight() + space);
        }
        return true;
    }

private:
    Layout::Hierarchy _hierarchy;
    LayoutParam::Align _align_items;
};

}

VerticalLayout::VerticalLayout(LayoutParam::Align alignItems)
    : _align_items(alignItems)
{
}

sp<Updatable> VerticalLayout::inflate(Hierarchy hierarchy)
{
    return sp<Updatable>::make<UpdatableVerticalLayout>(std::move(hierarchy), _align_items);
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
