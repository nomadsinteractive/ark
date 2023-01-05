#include "app/view/layout_hierarchy.h"

#include "core/epi/disposed.h"
#include "core/epi/visibility.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v4.h"
#include "graphics/inf/renderer.h"

#include "app/base/event.h"
#include "app/inf/layout.h"
#include "app/view/view.h"
#include "app/view/view_group.h"

namespace ark {

LayoutHierarchy::Slot::Slot(const sp<Renderer>& renderer, bool layoutRequested)
    : _layout_requested(layoutRequested), _renderer(renderer), _view(_renderer.as<View>()), _view_group(_renderer.as<ViewGroup>()),
      _layout_event_listener(renderer.as<LayoutEventListener>()), _disposed(renderer.as<Disposed>()), _visible(_renderer.as<Visibility>())
{
    DASSERT(_renderer);
}

void LayoutHierarchy::Slot::traverse(const Holder::Visitor& visitor)
{
    if(_view)
        HolderUtil::visit(_view, visitor);
    else
        HolderUtil::visit(_renderer, visitor);
}

void LayoutHierarchy::Slot::updateLayoutPosition(const V2& position, float clientHeight)
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        DASSERT(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            const V4 margins = layoutParam->margins().val();
            _position = V2(position.x() + margins.w(), clientHeight - layoutParam->contentHeight() - position.y() - margins.x());
        }
    }
}

bool LayoutHierarchy::Slot::isDisposed() const
{
    return _disposed && _disposed->val();
}

bool LayoutHierarchy::Slot::isVisible() const
{
    return !_visible || _visible->val();
}

bool LayoutHierarchy::Slot::layoutRequested() const
{
    return _layout_requested;
}

void LayoutHierarchy::Slot::updateLayout()
{
    _layout_requested = false;
}

void LayoutHierarchy::Slot::wrapContentLayout() const
{
    if(_view_group && _view_group->layoutParam()->isWrapContent())
        _view_group->updateLayout();
}

void LayoutHierarchy::Slot::render(RenderRequest& renderRequest, const V3& position)
{
    if(!_layout_requested)
    {
        _renderer->render(renderRequest, position + V3(_position, 0));
        if(_view)
            _layout_requested = VariableUtil::update(renderRequest.timestamp(), _view->layoutParam()->margins(), _view->size());
    }
}

bool LayoutHierarchy::Slot::onEventDispatch(const Event& event, float x, float y)
{
    if(_view && isVisible())
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        const V2 pos = _position;
        const Rect target(x + pos.x(), y + pos.y(), x + pos.x() + layoutParam->contentWidth(), y + pos.y() + layoutParam->contentHeight());
        const Event viewEvent(event, V2(event.x() - x - pos.x(), event.y() - y - pos.y()));
        const bool ptin = event.ptin(target);
        if(_layout_event_listener)
            return _layout_event_listener->onEvent(event, target.left(), target.top(), ptin);
        return _view->dispatchEvent(viewEvent, ptin);
    }
    return false;
}

sp<LayoutParam> LayoutHierarchy::Slot::getLayoutParam() const
{
    return _view ? static_cast<sp<LayoutParam>>(_view->layoutParam()) : nullptr;
}

sp<LayoutV3::Node> LayoutHierarchy::Slot::makeLayoutNode() const
{
    if(_view_group || _view)
        return _view_group ? _view_group->makeLayoutNode() : sp<LayoutV3::Node>::make(_view->layoutParam());
    return nullptr;
}

LayoutHierarchy::LayoutHierarchy(sp<Layout> layout, sp<LayoutV3> layoutV3)
    : _layout(std::move(layout)), _layout_v3(std::move(layoutV3))
{
}

void LayoutHierarchy::traverse(const Holder::Visitor& visitor)
{
    for(const sp<Slot>& i : _slots)
        i->traverse(visitor);
    for(const sp<Slot>& i : _incremental)
        i->traverse(visitor);
}

void LayoutHierarchy::render(RenderRequest& renderRequest, const V3& position) const
{
    for(const sp<Slot>& i: _slots)
        if(i->isVisible())
            i->render(renderRequest, position);
}

bool LayoutHierarchy::onEvent(const Event& event, float x, float y) const
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

bool LayoutHierarchy::isLayoutNeeded(const LayoutParam& layoutParam, bool& inflateNeeded)
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

std::pair<std::vector<sp<LayoutHierarchy::Slot>>, std::vector<sp<LayoutParam>>> LayoutHierarchy::getLayoutItems() const
{
    std::pair<std::vector<sp<LayoutHierarchy::Slot>>, std::vector<sp<LayoutParam>>> items = std::make_pair(std::vector<sp<LayoutHierarchy::Slot>>(), std::vector<sp<LayoutParam>>());
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

void LayoutHierarchy::updateLayout(const sp<LayoutParam>& layoutParam)
{
    bool inflatNeeded = false;
    if(_incremental.size())
    {
        for(sp<Slot>& i : _incremental)
            _slots.push_back(std::move(i));
        _incremental.clear();
        inflatNeeded = true;
    }

    if(isLayoutNeeded(layoutParam, inflatNeeded))
    {
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

            const std::vector<V2> positions = _layout->place(layoutParamSlots, layoutParam, contentSize);
            DASSERT(positions.size() == slots.size());

            for(size_t i = 0; i < positions.size(); ++i)
                slots.at(i)->updateLayoutPosition(positions.at(i), layoutParam->contentHeight());
        }
        else if(_layout_v3)
        {
            if(inflatNeeded)
                doLayoutInflat(layoutParam);

            _layout_v3->place(_layout_node);
        }

        for(const sp<Slot>& i : _slots)
            i->updateLayout();
    }
}

void LayoutHierarchy::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(renderer);
    _incremental.push_back(sp<LayoutHierarchy::Slot>::make(renderer, static_cast<bool>(_layout)));
}

sp<LayoutV3::Node> LayoutHierarchy::makeLayoutNode(sp<LayoutParam> layoutParam) const
{
    sp<LayoutV3::Node> node = sp<LayoutV3::Node>::make(std::move(layoutParam));
    for(const Slot& i : _slots)
        node->_child_nodes.push_back(i.makeLayoutNode());
    return node;
}

void LayoutHierarchy::doLayoutInflat(sp<LayoutParam> layoutParam)
{
    _layout_node = makeLayoutNode(std::move(layoutParam));
    _layout_v3->inflate(_layout_node);
}

}
