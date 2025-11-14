#include "app/view/view_hierarchy.h"

#include "graphics/base/render_request.h"
#include "graphics/inf/layout.h"
#include "graphics/components/layout_param.h"

#include "app/view/view.h"

namespace ark {

ViewHierarchy::ViewHierarchy(sp<Layout> layout)
    : _layout(std::move(layout))
{
}

bool ViewHierarchy::isLayoutTopView() const
{
    return static_cast<bool>(_layout);
}

bool ViewHierarchy::updateDescendantLayout(const uint32_t tick)
{
    bool isDirty = false;
    for(const sp<View>& i: updateChildren())
        isDirty = i->update(tick) || isDirty;
    return isDirty;
}

bool ViewHierarchy::updateHierarchy()
{
    bool hierarchyChanged = !_incremental.empty();
    if(hierarchyChanged)
        updateChildren();

    for(auto iter = _children.begin(); iter != _children.end(); )
    {
        if(const sp<View>& i = *iter; i->discarded().val())
        {
            iter = _children.erase(iter);
            hierarchyChanged = true;
        }
        else
        {
            if(i->hierarchy())
                hierarchyChanged |= i->hierarchy()->updateHierarchy();

            ++iter;
        }
    }
    return hierarchyChanged;
}

Layout::Hierarchy ViewHierarchy::toLayoutHierarchy(sp<Layout::Node> layoutNode) const
{
    Layout::Hierarchy hierarchy{std::move(layoutNode)};
    for(const sp<View>& i : _children)
    {
        const sp<ViewHierarchy>& viewHierarchy = i->hierarchy();
        hierarchy._child_nodes.push_back(viewHierarchy && !viewHierarchy->_layout ? viewHierarchy->toLayoutHierarchy(i->layoutNode()) : Layout::Hierarchy{i->layoutNode()});
    }
    return hierarchy;
}

bool ViewHierarchy::updateLayout(const sp<Layout::Node>& layoutNode, const uint32_t tick)
{
    bool hierarchyDirty = _timestamp.update(tick);
    if(const bool hierarchyChanged = updateHierarchy(); hierarchyChanged || hierarchyDirty)
    {
        if(_layout && (hierarchyChanged || !_updatable_layout))
        {
            _updatable_layout = _layout->inflate(toLayoutHierarchy(layoutNode));
            _timestamp.markClean();
        }
        hierarchyDirty = true;
    }
    if(_updatable_layout)
        return _updatable_layout->update(tick) || hierarchyDirty;

    return updateDescendantLayout(tick) || hierarchyDirty;
}

const Vector<sp<View>>& ViewHierarchy::updateChildren()
{
    for(sp<View>& i : _incremental)
        _children.push_back(std::move(i));
    _incremental.clear();
    return _children;
}

void ViewHierarchy::markHierarchyDirty()
{
    _timestamp.markDirty();
}

void ViewHierarchy::addView(sp<View> view)
{
    ASSERT(view);
    _incremental.push_back(std::move(view));
}

}
