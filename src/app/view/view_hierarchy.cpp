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

bool ViewHierarchy::updateDescendantLayout(uint64_t timestamp)
{
    bool isDirty = false;
    for(const sp<View>& i: updateChildren())
        if(i->visible().val())
            isDirty = i->updateLayout(timestamp) || isDirty;
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
                i->hierarchy()->updateHierarchy();

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

bool ViewHierarchy::updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool layoutParamDirty)
{
    const bool hierarchyDirty = _timestamp.update(timestamp);
    if(const bool hierarchyChanged = updateHierarchy() || hierarchyDirty; hierarchyChanged || layoutParamDirty)
    {
        if(_layout)
        {
            if(hierarchyChanged)
            {
                _updatable_layout = _layout->inflate(toLayoutHierarchy(layoutNode));
                _timestamp.markClean();
                timestamp = 0;
            }

            if(_updatable_layout)
                _updatable_layout->update(timestamp);
        }
        layoutParamDirty = true;
    }

    return updateDescendantLayout(timestamp) || layoutParamDirty;
}

const std::vector<sp<View>>& ViewHierarchy::updateChildren()
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
