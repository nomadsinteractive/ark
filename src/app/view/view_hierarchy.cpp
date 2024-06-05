#include "app/view/view_hierarchy.h"

#include "core/util/holder_util.h"

#include "graphics/base/render_request.h"

#include "app/inf/layout.h"
#include "app/traits/layout_param.h"
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

bool ViewHierarchy::isInflateNeeded()
{
    bool inflateNeeded = !_incremental.empty();
    if(inflateNeeded)
        updateSlots();

    for(auto iter = _slots.begin(); iter != _slots.end(); )
    {
        const sp<View>& i = *iter;
        if(i->discarded().val())
        {
            iter = _slots.erase(iter);
            inflateNeeded = true;
        }
        else
            ++iter;
    }
    return inflateNeeded;
}

std::vector<sp<View>> ViewHierarchy::getLayoutItems() const
{
    std::vector<sp<View>> items;
    for(const sp<View>& i : _slots)
        if(sp<LayoutParam> lp = i->layoutParam(); lp && lp->display() == LayoutParam::DISPLAY_BLOCK)
            items.push_back(i);
    return items;
}

bool ViewHierarchy::updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool isDirty)
{
    if(const bool inflateNeeded = isInflateNeeded(); inflateNeeded || isDirty)
    {
        if(_layout)
        {
            if(inflateNeeded)
            {
                _layout->inflate(layoutNode);
                timestamp = 0;
            }

            _layout->update(timestamp);
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

const std::vector<sp<View>>& ViewHierarchy::updateSlotsAndLayoutNodes()
{
    for(const View& i : updateSlots())
    {
        const sp<Layout::Node>& layoutNode = i.layoutNode();
        layoutNode->setPaddings(i.layoutParam()->paddings().val());
        layoutNode->setMargins(i.layoutParam()->margins().val());
    }
    return _slots;
}

void ViewHierarchy::addView(sp<View> view)
{
    ASSERT(view);
    _incremental.push_back(sp<View>::make(std::move(view)));
}

}
