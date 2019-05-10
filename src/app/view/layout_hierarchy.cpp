#include "app/view/layout_hierarchy.h"

#include "core/epi/disposed.h"
#include "core/epi/visibility.h"

#include "graphics/base/size.h"
#include "graphics/inf/renderer.h"

#include "app/base/event.h"
#include "app/inf/layout.h"
#include "app/view/view.h"
#include "app/view/view_group.h"


namespace ark {

LayoutHierarchy::Slot::Slot(const sp<Renderer>& renderer, bool layoutRequested)
    : _x(0), _y(0), _layout_width(0), _layout_height(0), _layout_requested(layoutRequested), _renderer(renderer), _view(renderer.as<View>()), _view_group(renderer.as<ViewGroup>()),
      _disposed(renderer.as<Disposed>()), _visibility(renderer.as<Visibility>())
{
    DASSERT(renderer);
}

bool LayoutHierarchy::Slot::isDisposed() const
{
    return _disposed && _disposed->isDisposed();
}

bool LayoutHierarchy::Slot::layoutRequested() const
{
    return _layout_requested;
}

void LayoutHierarchy::Slot::updateLayout()
{
    if(_view)
    {
        _layout_width = _view->size()->width();
        _layout_height = _view->size()->height();
    }
    _layout_requested = false;
}

void LayoutHierarchy::Slot::doPlace(float clientHeight, const sp<Layout>& layout)
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        DASSERT(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            const Rect& margins = layoutParam->margins();
            const Rect target = layout->place(layoutParam);
            _y = clientHeight - target.top() - margins.top() - layoutParam->contentHeight();
            _x = target.left() + margins.left();
        }
    }
}

void LayoutHierarchy::Slot::doWrapContentPlace(const sp<Layout>& layout, Rect& contentRect) const
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        DASSERT(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            const Rect rect = layout->place(layoutParam);
            contentRect.setLeft(std::min(contentRect.left(), rect.left()));
            contentRect.setTop(std::min(contentRect.top(), rect.top()));
            contentRect.setRight(std::max(contentRect.right(), rect.right()));
            contentRect.setBottom(std::max(contentRect.bottom(), rect.bottom()));
        }
    }
}

void LayoutHierarchy::Slot::doLayoutEnd(const Rect& p)
{
    if(_view)
    {
        _x += p.left();
        _y -= (g_upDirection * p.top());
    }
    updateLayout();
}

void LayoutHierarchy::Slot::render(RenderRequest& renderRequest, float x, float y)
{
    if(!_layout_requested)
    {
        _renderer->render(renderRequest, x + _x, y + _y);
        if(_view)
            _layout_requested = _layout_width != _view->size()->width() || _layout_height != _view->size()->height();
    }
}

bool LayoutHierarchy::Slot::onEventDispatch(const Event& event, float x, float y)
{
    if(_view && (!_visibility || _visibility->visible()))
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        const Rect target(x + _x, y + _y, x + _x + layoutParam->contentWidth(), y + _y + layoutParam->contentHeight());
        const Event viewEvent(event.action(), event.x() - _x - x, event.y() - _y - y, event.timestamp(), event.code());
        if(_view_group)
            return _view_group->dispatchEvent(viewEvent, event.ptin(target)) || _view_group->onEvent(event, target.left(), target.top());
        return _view->dispatchEvent(viewEvent, event.ptin(target));
    }
    return false;
}

LayoutHierarchy::LayoutHierarchy(const sp<Layout>& layout)
    : _layout(layout)
{
}

void LayoutHierarchy::render(RenderRequest& renderRequest, float x, float y) const
{
    for(const sp<Slot>& i: _slots)
        i->render(renderRequest, x, y);
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

bool LayoutHierarchy::isLayoutNeeded(const LayoutParam& layoutParam)
{
    bool layoutNeeded = false;
    for(auto iter = _slots.begin(); iter != _slots.end(); ++iter)
    {
        const sp<Slot>& i = *iter;
        if(i->isDisposed())
        {
            iter = _slots.erase(iter);
            if(iter == _slots.end())
                return true;
            continue;
        }
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

void LayoutHierarchy::updateLayout(LayoutParam& layoutParam)
{
    if(_incremental.size())
    {
        for(const sp<Slot>& i : _incremental)
            _slots.push_back(i);
        _incremental.clear();
    }
    if(isLayoutNeeded(layoutParam))
    {
        if(_layout)
        {
            if(layoutParam.isWrapContent())
                doWrapContentLayout(layoutParam);

            _layout->begin(layoutParam);

            for(const sp<Slot>& i: _slots)
                i->doPlace(layoutParam.contentHeight(), _layout);

            const Rect p = _layout->end();
            for(const sp<Slot>& i : _slots)
                i->doLayoutEnd(p);
        }
        else
            for(const sp<Slot>& i : _slots)
                i->updateLayout();
    }
}

void LayoutHierarchy::doWrapContentLayout(LayoutParam& layoutParam)
{
    LayoutParam lp(layoutParam);
    Rect clientRect;
    _layout->begin(lp);
    for(const sp<Slot>& i: _slots)
        i->doWrapContentPlace(_layout, clientRect);

    _layout->end();
    layoutParam.setContentWidth(clientRect.width());
    layoutParam.setContentHeight(clientRect.height());
}

void LayoutHierarchy::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(renderer);
    _incremental.push_back(sp<LayoutHierarchy::Slot>::make(renderer, static_cast<bool>(_layout)));
}

}
