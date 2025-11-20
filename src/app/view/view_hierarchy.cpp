#include "app/view/view_hierarchy.h"

#include "graphics/base/render_request.h"
#include "graphics/inf/layout.h"
#include "graphics/components/layout_param.h"

#include "app/view/view.h"

namespace ark {

namespace {

sp<Layout> getTopViewLayout(const sp<View>& view)
{
    if(view->hierarchy() && view->hierarchy()->layout())
        return view->hierarchy()->layout();
    if(const sp<View> parent = view->parent())
        return getTopViewLayout(parent);
    return nullptr;
}

}

ViewHierarchy::ViewHierarchy(sp<Layout> layout, sp<Layout::Node> layoutNode)
    : _layout(std::move(layout)), _layout_node(std::move(layoutNode))
{
}

bool ViewHierarchy::isLayoutTopView() const
{
    return static_cast<bool>(_layout);
}

const sp<Layout>& ViewHierarchy::layout() const
{
    return _layout;
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
    bool hierarchyChanged = false;
    updateChildren();
    for(auto iter = _children.begin(); iter != _children.end(); )
    {
        if(const sp<View>& i = *iter; i->discarded().val())
        {
            const sp<Layout> layout = getTopViewLayout(i);
            hierarchyChanged = layout ? !layout->removeNode(i->layoutNode()) : true;
            iter = _children.erase(iter);
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

Layout::Hierarchy ViewHierarchy::toLayoutHierarchy() const
{
    Layout::Hierarchy hierarchy{_layout_node};
    for(const sp<View>& i : _children)
    {
        const sp<ViewHierarchy>& viewHierarchy = i->hierarchy();
        hierarchy._child_nodes.push_back(viewHierarchy && !viewHierarchy->_layout ? viewHierarchy->toLayoutHierarchy() : Layout::Hierarchy{i->layoutNode()});
    }
    return hierarchy;
}

bool ViewHierarchy::updateLayout(const uint32_t tick)
{
    const bool hierarchyChanged = updateHierarchy();
    const bool hierarchyDirty = _timestamp.update(tick);
    if(hierarchyChanged || hierarchyDirty)
    {
        if(_layout)
        {
            _updatable_layout = _layout->inflate(toLayoutHierarchy());
            _timestamp.markClean();
        }
    }
    if(_updatable_layout)
        return _updatable_layout->update(tick) || hierarchyDirty;

    return updateDescendantLayout(tick) || hierarchyChanged || hierarchyDirty;
}

const Vector<sp<View>>& ViewHierarchy::updateChildren()
{
    if(!_incremental.empty())
    {
        if(const sp<Layout> layout = getTopViewLayout(_incremental.at(0)))
        {
            for(const sp<View>& i : _incremental)
                if(!layout->appendNode(_layout_node, i))
                {
                    _timestamp.markDirty();
                    break;
                }
        }
        else
            _timestamp.markDirty();
        for(sp<View>& i : _incremental)
            _children.push_back(std::move(i));
        _incremental.clear();
    }
    return _children;
}

void ViewHierarchy::markHierarchyDirty()
{
    if(_layout)
        _timestamp.markDirty();
}

void ViewHierarchy::addView(sp<View> view)
{
    ASSERT(view);
    _incremental.push_back(std::move(view));
}

}
