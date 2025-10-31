#include "app/impl/layout/horizontal_layout.h"

#include "app/util/layout_util.h"
#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark {

namespace {

class UpdatableHorizontalLayout : public Updatable {
public:
    UpdatableHorizontalLayout(Layout::Hierarchy hierarchy, LayoutParam::Align alignItems)
        : _hierarchy(std::move(hierarchy)), _align_items(alignItems) {
    }

    bool update(uint32_t tick) override {
        float totalWidth = 0;
        for(const auto& [node, child_nodes] : _hierarchy._child_nodes) {
            DCHECK_WARN(child_nodes.empty(), "HorizontalLayout is not a recursively layout for now");
            node->update(tick);
            totalWidth += node->occupyWidth();
        }

        const Layout::Node& rootNode = *_hierarchy._node;
        const V2 layoutOffsetPos = rootNode.offsetPosition();
        LayoutParam::JustifyContent justifyContent = rootNode._layout_param->justifyContent();
        const auto [start, space] = LayoutUtil::calcFlowDirection(justifyContent, rootNode.contentWidth(), totalWidth, _hierarchy._child_nodes.size());
        float x = start;
        for(const auto& [node, child_nodes] : _hierarchy._child_nodes) {
            Layout::Node& layoutNode = *node;
            float y = LayoutUtil::calcItemOffsetY(_align_items, rootNode, layoutNode);
            layoutNode.setOffsetPosition(layoutOffsetPos + V2(x, y));
            x += (layoutNode.occupyWidth() + space);
        }
        return true;
    }

private:
    Layout::Hierarchy _hierarchy;
    LayoutParam::Align _align_items;
};

}

HorizontalLayout::HorizontalLayout(LayoutParam::Align alignItems)
    : _align_items(alignItems)
{
}

sp<Updatable> HorizontalLayout::inflate(Hierarchy hierarchy)
{
    return sp<Updatable>::make<UpdatableHorizontalLayout>(std::move(hierarchy), _align_items);
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
