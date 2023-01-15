#include "app/view/view_hierarchy.h"

#include "core/epi/disposed.h"
#include "core/epi/visibility.h"
#include "core/util/holder_util.h"
#include "core/util/updatable_util.h"
#include "core/util/updatable_util.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"
#include "graphics/inf/renderer.h"

#include "app/base/event.h"
#include "app/inf/layout.h"
#include "app/view/layout_param.h"
#include "app/view/view.h"

namespace ark {

ViewHierarchy::Slot::Slot(const sp<Renderer>& renderer, sp<View> view, bool layoutRequested)
    : _layout_requested(layoutRequested), _renderer(renderer), _view(std::move(view)), _layout_event_listener(renderer.as<LayoutEventListener>()),
      _disposed(_renderer.as<Disposed>()), _visible(_renderer.as<Visibility>())
{
    if(!_view)
        _view = _renderer.as<View>();
}

void ViewHierarchy::Slot::traverse(const Holder::Visitor& visitor)
{
    if(_view)
        HolderUtil::visit(_view, visitor);
    else
        HolderUtil::visit(_renderer, visitor);
}

const sp<View>& ViewHierarchy::Slot::view() const
{
    return _view;
}

void ViewHierarchy::Slot::updateLayoutPosition(const V2& position, float clientHeight)
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        DASSERT(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            LayoutV3::Node& layoutNode = _view->layoutNode();
            const V4 margins = layoutParam->margins().val();
            layoutNode._offset_position = V2(position.x() + margins.w(), clientHeight - layoutParam->contentHeight() - position.y() - margins.x());
        }
    }
}

bool ViewHierarchy::Slot::isDisposed() const
{
    return _disposed && _disposed->val();
}

bool ViewHierarchy::Slot::isVisible() const
{
    return !_visible || _visible->val();
}

bool ViewHierarchy::Slot::layoutRequested() const
{
    return _layout_requested;
}

void ViewHierarchy::Slot::updateLayout()
{
    _layout_requested = false;
}

void ViewHierarchy::Slot::wrapContentLayout() const
{
    if(_view && _view->layoutParam()->isWrapContent())
        _view->updateLayout();
}

void ViewHierarchy::Slot::render(RenderRequest& renderRequest, const V3& position)
{
    if(!_layout_requested)
    {
        if(_renderer)
            _renderer->render(renderRequest, _view ? (position + V3(_view->layoutNode()->_offset_position, 0)) : position);
        if(_view)
            _layout_requested = UpdatableUtil::update(renderRequest.timestamp(), _view->layoutParam()->margins(), _view->size());
    }
}

bool ViewHierarchy::Slot::onEventDispatch(const Event& event, float x, float y)
{
    if(_view && isVisible())
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        const V2 pos = _view->layoutNode()->_offset_position;
        const Rect target(x + pos.x(), y + pos.y(), x + pos.x() + layoutParam->contentWidth(), y + pos.y() + layoutParam->contentHeight());
        const Event viewEvent(event, V2(event.x() - x - pos.x(), event.y() - y - pos.y()));
        const bool ptin = event.ptin(target);
        if(_layout_event_listener)
            return _layout_event_listener->onEvent(event, target.left(), target.top(), ptin);
        return _view->dispatchEvent(viewEvent, ptin);
    }
    return false;
}

sp<LayoutParam> ViewHierarchy::Slot::getLayoutParam() const
{
    return _view ? _view->layoutParam() : nullptr;
}

sp<LayoutV3::Node> ViewHierarchy::Slot::layoutNode() const
{
    return _view ? _view->layoutNode() : nullptr;
}

ViewHierarchy::ViewHierarchy(sp<Layout> layout, sp<LayoutV3> layoutV3)
    : _layout(std::move(layout)), _layout_v3(std::move(layoutV3))
{
}

void ViewHierarchy::traverse(const Holder::Visitor& visitor)
{
    for(const sp<Slot>& i : _slots)
        i->traverse(visitor);
    for(const sp<Slot>& i : _incremental)
        i->traverse(visitor);
}

bool ViewHierarchy::update(uint64_t timestamp)
{
    bool dirty = _incremental.size() > 0;
    for(const sp<Slot>& i : _slots)
        dirty = i->_view->_stub->update(timestamp) || dirty;
    return dirty;
}

const sp<LayoutV3>& ViewHierarchy::layout() const
{
    return _layout_v3;
}

void ViewHierarchy::render(RenderRequest& renderRequest, const V3& position)
{
    for(Slot& i: updateSlots())
        if(i.isVisible())
            i.render(renderRequest, position);
}

bool ViewHierarchy::onEvent(const Event& event, float x, float y) const
{
    Event::Action action = event.action();
    if(action == Event::ACTION_MOVE || action == Event::ACTION_UP || action == Event::ACTION_DOWN)
    {
        for(auto iter =_slots.rbegin(); iter != _slots.rend(); ++iter)
            if((*iter)->onEventDispatch(event, x, y))
                return true;
    }
    return false;
}

bool ViewHierarchy::isLayoutNeeded(const LayoutParam& layoutParam, bool& inflateNeeded)
{
    bool layoutNeeded = false;
    for(auto iter = _slots.begin(); iter != _slots.end(); )
    {
        const sp<Slot>& i = *iter;
        if(i->isDisposed())
        {
            iter = _slots.erase(iter);
            layoutNeeded = true;
            inflateNeeded = true;
        }
        else
            ++iter;
        layoutNeeded = layoutNeeded || i->layoutRequested();
    }

    const V3 newLayoutSize = layoutParam.size()->val();
    if(newLayoutSize != _layout_size)
    {
        _layout_size = newLayoutSize;
        return true;
    }
    return layoutNeeded;
}

std::pair<std::vector<sp<ViewHierarchy::Slot>>, std::vector<sp<LayoutParam>>> ViewHierarchy::getLayoutItems() const
{
    std::pair<std::vector<sp<ViewHierarchy::Slot>>, std::vector<sp<LayoutParam>>> items = std::make_pair(std::vector<sp<ViewHierarchy::Slot>>(), std::vector<sp<LayoutParam>>());
    for(const sp<Slot>& i : _slots)
    {
        sp<LayoutParam> lp = i->getLayoutParam();
        if(lp && lp->display() == LayoutParam::DISPLAY_BLOCK)
        {
            items.first.push_back(i);
            items.second.push_back(std::move(lp));
        }
    }
    return items;
}

void ViewHierarchy::updateLayout(View& view, uint64_t timestamp, bool isDirty)
{
    bool inflatNeeded = _incremental.size() > 0;
    if(inflatNeeded)
        updateSlots();

    const sp<LayoutParam>& layoutParam = view.layoutParam();
    if(isLayoutNeeded(layoutParam, inflatNeeded) || isDirty)
    {
        sp<LayoutV3::Node> layoutNode = view.layoutNode();

        for(const sp<Slot>& i : _slots)
            i->wrapContentLayout();

        if(_layout)
        {
            const auto [slots, layoutParamSlots] = getLayoutItems();
            const V2 contentSize = _layout->inflate(layoutParamSlots);

            if(layoutParam->isWidthWrapContent())
                layoutParam->setContentWidth(contentSize.x());
            if(layoutParam->isHeightWrapContent())
                layoutParam->setContentHeight(contentSize.y());

            layoutNode->_size = layoutParam->size()->val();
            const std::vector<V2> positions = _layout->place(layoutParamSlots, layoutParam, contentSize);
            DASSERT(positions.size() == slots.size());

            for(size_t i = 0; i < positions.size(); ++i)
                slots.at(i)->updateLayoutPosition(positions.at(i), layoutParam->contentHeight());
        }
        else if(_layout_v3)
        {
            if(inflatNeeded)
            {
                _layout_v3->inflate(layoutNode);
                timestamp = 0;
            }

            _layout_v3->update(timestamp);
        }

        for(const sp<Slot>& i : _slots)
            i->updateLayout();
    }
}

const std::vector<sp<ViewHierarchy::Slot>>& ViewHierarchy::updateSlots()
{
    for(sp<Slot>& i : _incremental)
        _slots.push_back(std::move(i));
    _incremental.clear();
    return _slots;
}

void ViewHierarchy::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(renderer);
    _incremental.push_back(sp<ViewHierarchy::Slot>::make(renderer, renderer.as<View>(), static_cast<bool>(_layout) || static_cast<bool>(_layout_v3)));
}

void ViewHierarchy::addView(sp<View> view)
{
    ASSERT(view);
    sp<Renderer> renderer = view;
    _incremental.push_back(sp<ViewHierarchy::Slot>::make(std::move(renderer), std::move(view), static_cast<bool>(_layout) || static_cast<bool>(_layout_v3)));
}

}
