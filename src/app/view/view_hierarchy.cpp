#include "app/view/view_hierarchy.h"

#include "graphics/base/render_request.h"

#include "graphics/inf/layout.h"
#include "graphics/traits/layout_param.h"
#include "app/view/view.h"

namespace ark {

ViewHierarchy::ViewHierarchy(sp<Layout> layout)
    : _layout(std::move(layout))
{
}

bool ViewHierarchy::isIsolatedLayout() const
{
    return static_cast<bool>(_layout);
}

bool ViewHierarchy::updateDescendantLayout(uint64_t timestamp)
{
    bool isDirty = false;
    for(const sp<View>& i: updateSlots())
        if(i->visible().val())
            isDirty = i->updateLayout(timestamp) || isDirty;
    return isDirty;
}

bool ViewHierarchy::updateHierarchy()
{
    bool hierarchyChanged = !_incremental.empty();
    if(hierarchyChanged)
        updateSlots();

    for(auto iter = _slots.begin(); iter != _slots.end(); )
    {
        const sp<View>& i = *iter;
        if(i->discarded().val())
        {
            iter = _slots.erase(iter);
            hierarchyChanged = true;
        }
        else
            ++iter;
    }
    return hierarchyChanged;
}

Layout::Hierarchy ViewHierarchy::toLayoutHierarchy(sp<Layout::Node> layoutNode) const
{
    Layout::Hierarchy hierarchy{std::move(layoutNode)};
    for(const sp<View>& i : _slots)
    {
        const sp<ViewHierarchy>& viewHierarchy = i->hierarchy();
        hierarchy._child_nodes.push_back(viewHierarchy && !viewHierarchy->_layout ? viewHierarchy->toLayoutHierarchy(i->layoutNode()) : Layout::Hierarchy{i->layoutNode()});
    }
    return hierarchy;
}

bool ViewHierarchy::updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool isDirty)
{
    if(const bool hierarchyChanged = updateHierarchy(); hierarchyChanged || isDirty)
    {
        if(_layout)
        {
            if(hierarchyChanged)
            {
                _updatable_layout = _layout->inflate(toLayoutHierarchy(layoutNode));
                timestamp = 0;
            }

            if(_updatable_layout)
                _updatable_layout->update(timestamp);
        }
        isDirty = true;
    }

    return updateDescendantLayout(timestamp) || isDirty;
}

const std::vector<sp<View>>& ViewHierarchy::updateSlots()
{
    for(sp<View>& i : _incremental)
        _slots.push_back(std::move(i));
    _incremental.clear();
    return _slots;
}

void ViewHierarchy::addView(sp<View> view)
{
    ASSERT(view);
    _incremental.push_back(std::move(view));
}

}
