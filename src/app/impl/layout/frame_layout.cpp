#include "app/impl/layout/frame_layout.h"

#include "graphics/components/layout_param.h"

#include "app/util/layout_util.h"

namespace ark {

namespace {

class UpdatableFrameLayout final : public Updatable {
public:
    UpdatableFrameLayout(Layout::Hierarchy hierarchy)
        : _hierarchy(std::move(hierarchy)) {
    }

    bool update(uint64_t timestamp) override {
        bool dirty = false;
        const Layout::Node& parentNode = _hierarchy._node;
        for(const Layout::Hierarchy& i : _hierarchy._child_nodes)
            if(i._node->update(timestamp))
            {
                i._node->setSize(LayoutUtil::calcItemSize(i._node->_layout_param, parentNode), false);

                float offsetX = LayoutUtil::calcItemOffsetX(LayoutUtil::toAlign(i._node->_layout_param->justifyContent()), parentNode, i._node);
                float offsetY = LayoutUtil::calcItemOffsetY(i._node->_layout_param->alignSelf(), parentNode, i._node);
                i._node->setOffsetPosition({offsetX, offsetY}, false);
                dirty = true;
            }
        return dirty;
    }

private:
    Layout::Hierarchy _hierarchy;
};

}

sp<Updatable> FrameLayout::inflate(Hierarchy hierarchy)
{
    return sp<Updatable>::make<UpdatableFrameLayout>(std::move(hierarchy));
}

sp<Layout> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::make<FrameLayout>();
}

}
