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
    : _x(0), _y(0), _layout_width(0), _layout_height(0), _layout_requested(layoutRequested), _renderer(renderer), _view(renderer.as<View>()), _view_group(renderer.as<ViewGroup>()),
      _layout_event_listener(renderer.as<LayoutEventListener>()), _disposed(renderer.as<Disposed>()), _visibility(renderer.as<Visibility>())
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

bool LayoutHierarchy::Slot::isDisposed() const
{
    return _disposed && _disposed->val();
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

void LayoutHierarchy::Slot::wrapContentLayout() const
{
    if(_view_group && _view_group->layoutParam()->isWrapContent())
        _view_group->updateLayout();
}

void LayoutHierarchy::Slot::doPlace(Layout::Context& ctx, float clientHeight, const sp<Layout>& layout)
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        DASSERT(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            const V4 margins = layoutParam->margins().val();
            const Rect target = layout->place(ctx, layoutParam);
            _y = clientHeight - layoutParam->contentHeight() - target.top() - margins.x();
            _x = target.left() + margins.w();
        }
    }
}

void LayoutHierarchy::Slot::doWrapContentPlace(Layout::Context& ctx, const sp<Layout>& layout, Rect& contentRect) const
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        DASSERT(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            const Rect rect = layout->place(ctx, layoutParam);
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
        _y -= p.top();
    }
}

void LayoutHierarchy::Slot::render(RenderRequest& renderRequest, const V3& position)
{
    if(!_layout_requested)
    {
        _renderer->render(renderRequest, position + V3(_x, _y, 0));
        if(_view)
            _layout_requested = VariableUtil::update(renderRequest.timestamp(), _view->layoutParam()->margins(), _view->size());
    }
}

bool LayoutHierarchy::Slot::onEventDispatch(const Event& event, float x, float y)
{
    if(_view && (!_visibility || _visibility->val()))
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        const Rect target(x + _x, y + _y, x + _x + layoutParam->contentWidth(), y + _y + layoutParam->contentHeight());
        const Event viewEvent(event.action(), event.x() - _x - x, event.y() - _y - y, event.timestamp(), event.code());
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

LayoutHierarchy::LayoutHierarchy(const sp<Layout>& layout)
    : _layout(layout)
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

bool LayoutHierarchy::isLayoutNeeded(const LayoutParam& layoutParam)
{
    bool layoutNeeded = false;
    for(auto iter = _slots.begin(); iter != _slots.end(); )
    {
        const sp<Slot>& i = *iter;
        if(i->isDisposed())
        {
            iter = _slots.erase(iter);
            layoutNeeded = true;
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

std::vector<sp<LayoutParam>> LayoutHierarchy::getLayoutParams() const
{
    std::vector<sp<LayoutParam>> layoutParams;

    for(const sp<Slot>& i : _slots)
    {
        sp<LayoutParam> lp = i->getLayoutParam();
        if(lp && lp->display() == LayoutParam::DISPLAY_BLOCK)
            layoutParams.push_back(std::move(lp));
    }

    return layoutParams;
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
        for(const sp<Slot>& i : _slots)
            i->wrapContentLayout();

        if(_layout)
        {
            Layout::Context ctx(layoutParam, [this]() {
                return this->getLayoutParams();
            });

            if(layoutParam.isWrapContent())
                doWrapContentLayout(ctx, layoutParam);

            _layout->begin(ctx, layoutParam);

            for(const sp<Slot>& i: _slots)
                i->doPlace(ctx, layoutParam.contentHeight(), _layout);

            const Rect p = _layout->end(ctx);
            for(const sp<Slot>& i : _slots)
                i->doLayoutEnd(p);
        }
        for(const sp<Slot>& i : _slots)
            i->updateLayout();
    }
}

void LayoutHierarchy::doWrapContentLayout(Layout::Context& ctx, LayoutParam& layoutParam)
{
    LayoutParam lp(layoutParam);
    Rect clientRect;
    _layout->begin(ctx, lp);
    for(const sp<Slot>& i: _slots)
        i->doWrapContentPlace(ctx, _layout, clientRect);

    _layout->end(ctx);
    if(layoutParam.isWidthWrapContent())
        layoutParam.setContentWidth(clientRect.width());
    if(layoutParam.isHeightWrapContent())
        layoutParam.setContentHeight(clientRect.height());
}

void LayoutHierarchy::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(renderer);
    _incremental.push_back(sp<LayoutHierarchy::Slot>::make(renderer, static_cast<bool>(_layout)));
}

}
