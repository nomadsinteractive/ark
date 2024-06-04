#include "app/view/view_hierarchy.h"

#include "core/traits/expendable.h"
#include "core/traits/visibility.h"
#include "core/util/holder_util.h"
#include "core/util/updatable_util.h"
#include "core/util/updatable_util.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"
#include "graphics/inf/renderer.h"

#include "app/base/event.h"
#include "app/inf/layout.h"
#include "app/traits/layout_param.h"
#include "app/view/view.h"

namespace ark {

ViewHierarchy::Slot::Slot(sp<View> view)
    : _view(std::move(view))
{
    DASSERT(_view);
}

void ViewHierarchy::Slot::updateLayoutPosition(const V2& position)
{
    const LayoutParam& layoutParam = _view->layoutParam();
    if(layoutParam.display() == LayoutParam::DISPLAY_BLOCK)
    {
        Layout::Node& layoutNode = _view->layoutNode();
        const V4 margins = layoutParam.margins().val();
        layoutNode.setPaddings(layoutParam.paddings().val());
        layoutNode.setOffsetPosition(V2(position.x() + margins.w(), position.y() + margins.x()));
    }
}

bool ViewHierarchy::Slot::isDiscarded() const
{
    return _view->discarded().val();
}

bool ViewHierarchy::Slot::isVisible() const
{
    return _view->visible().val();
}

const sp<LayoutParam>& ViewHierarchy::Slot::layoutParam() const
{
    return _view->layoutParam();
}

const sp<Layout::Node>& ViewHierarchy::Slot::layoutNode() const
{
    return _view->layoutNode();
}

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
    for(const Slot& i: updateSlots())
        if(i.isVisible())
            isDirty = i._view->updateLayout(timestamp) || isDirty;
    return isDirty;
}

bool ViewHierarchy::isInflateNeeded()
{
    bool inflateNeeded = _incremental.size() > 0;
    if(inflateNeeded)
        updateSlots();

    for(auto iter = _slots.begin(); iter != _slots.end(); )
    {
        const sp<Slot>& i = *iter;
        if(i->isDiscarded())
        {
            iter = _slots.erase(iter);
            inflateNeeded = true;
        }
        else
            ++iter;
    }
    return inflateNeeded;
}

std::pair<std::vector<sp<ViewHierarchy::Slot>>, std::vector<sp<LayoutParam>>> ViewHierarchy::getLayoutItems() const
{
    std::pair<std::vector<sp<ViewHierarchy::Slot>>, std::vector<sp<LayoutParam>>> items = std::make_pair(std::vector<sp<ViewHierarchy::Slot>>(), std::vector<sp<LayoutParam>>());
    for(const sp<Slot>& i : _slots)
    {
        sp<LayoutParam> lp = i->layoutParam();
        if(lp && lp->display() == LayoutParam::DISPLAY_BLOCK)
        {
            items.first.push_back(i);
            items.second.push_back(std::move(lp));
        }
    }
    return items;
}

bool ViewHierarchy::updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool isDirty)
{
    bool inflateNeeded = isInflateNeeded();
    if(inflateNeeded || isDirty)
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

const std::vector<sp<ViewHierarchy::Slot>>& ViewHierarchy::updateSlots()
{
    for(sp<Slot>& i : _incremental)
        _slots.push_back(std::move(i));
    _incremental.clear();
    return _slots;
}

const std::vector<sp<ViewHierarchy::Slot>>& ViewHierarchy::updateSlotsAndLayoutNodes()
{
    for(const Slot& i : updateSlots())
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
    _incremental.push_back(sp<ViewHierarchy::Slot>::make(std::move(view)));
}

}
