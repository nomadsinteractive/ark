#include "app/view/view_group.h"

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/util/log.h"

#include "graphics/base/frame.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/size.h"
#include "graphics/impl/renderer/renderer_delegate.h"

#include "app/base/event.h"
#include "app/inf/layout.h"

namespace ark {

ViewGroup::Placement::Placement(const sp<Renderer>& renderer, bool layoutRequested)
    : _x(0), _y(0), _layout_requested(layoutRequested), _renderer(renderer), _view(renderer.as<View>()), _view_group(renderer.as<ViewGroup>()), _renderer_delegate(renderer.as<RendererDelegate>()), _expirable(renderer.as<Expired>())
{
    NOT_NULL(renderer);
}

bool ViewGroup::Placement::isExpired() const
{
    return _expirable && _expirable->val();
}

bool ViewGroup::Placement::layoutRequested() const
{
    return _layout_requested;
}

void ViewGroup::Placement::updateLayout()
{
    if(_view)
    {
        _layout_width = _view->size()->width();
        _layout_height = _view->size()->height();
    }
    _layout_requested = false;
}

void ViewGroup::Placement::doPlace(float clientHeight, const sp<Layout>& layout)
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        NOT_NULL(layoutParam);
        if(layoutParam->display() == LayoutParam::DISPLAY_BLOCK)
        {
            const Rect& margins = layoutParam->margins();
            const Rect target = layout->place(layoutParam);
            _y = g_isOriginBottom ? clientHeight - target.top() - margins.top() - layoutParam->contentHeight() : target.top() + margins.top();
            _x = target.left() + margins.left();
        }
    }
}

void ViewGroup::Placement::doWrapContentPlace(const sp<Layout>& layout, Rect& contentRect) const
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        NOT_NULL(layoutParam);
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

void ViewGroup::Placement::doLayoutEnd(const Rect& p)
{
    if(_view)
    {
        _x += p.left();
        _y -= (g_upDirection * p.top());
    }
    updateLayout();
}

void ViewGroup::Placement::render(RenderRequest& renderRequest, float x, float y)
{
    if(!_layout_requested)
    {
        _renderer->render(renderRequest, x + _x, y + _y);
        if(_view)
        {
            _layout_requested = _layout_width != _view->size()->width();
            _layout_requested = _layout_requested || _layout_height != _view->size()->height();
        }
    }
}

bool ViewGroup::Placement::onEventDispatch(const Event& event, float x, float y)
{
    if(_view)
    {
        const sp<LayoutParam>& layoutParam = _view->layoutParam();
        Rect target(x + _x, y + _y, x + _x + layoutParam->contentWidth(), y + _y + layoutParam->contentHeight());
        Event viewEvent(event.action(), event.x() - _x - x, event.y() - _y - y, event.code());
        if(_view_group)
            return _view_group->dispatchEvent(viewEvent, event.ptin(target)) || _view_group->onEvent(event, target.left(), target.top());
        else if(_renderer_delegate)
        {
            const sp<ViewGroup> viewGroup = _renderer_delegate->delegate().as<ViewGroup>();
            if(viewGroup)
                return viewGroup->dispatchEvent(viewEvent, event.ptin(target)) || viewGroup->onEvent(event, target.left(), target.top());
        }
        else
            return _view->dispatchEvent(viewEvent, event.ptin(target));
    }
    return false;
}

ViewGroup::ViewGroup(const Frame& background, const sp<Layout>& layout, const sp<LayoutParam>& layoutParam)
    : View(!layoutParam && background ? sp<LayoutParam>::make(background.size()) : layoutParam), _background(background.renderer()), _layout(layout)
{
    DCHECK(!_layout || _layout_param, "Null LayoutParam. This would happen if your ViewGroup has neither background or size defined.");
    if(layoutParam && background)
        if(background.size() != layoutParam->size())
            background.size()->adopt(layoutParam->size());
}

ViewGroup::~ViewGroup()
{
    LOGD("");
}

void ViewGroup::addRenderer(const sp<Renderer>& renderer)
{
    NOT_NULL(renderer);
    _placments.push_back(sp<Placement>::make(renderer, static_cast<bool>(_layout)));
}

void ViewGroup::render(RenderRequest& renderRequest, float x, float y)
{
    if(_background)
        _background->render(renderRequest, x, y);

    if(isLayoutNeeded())
        doLayout();

    for(const sp<Placement>& i: _placments)
        i->render(renderRequest, x, y);
}

bool ViewGroup::onEvent(const Event& event, float x, float y)
{
    Event::Action action = event.action();
    if(action == Event::ACTION_MOVE || action == Event::ACTION_UP || action == Event::ACTION_DOWN)
    {
        for(auto iter =_placments.items().rbegin(); iter != _placments.items().rend(); ++iter)
            if((*iter)->onEventDispatch(event, x, y))
                return true;
    }
    return false;
}

void ViewGroup::doLayout()
{
    if(_layout)
    {
        if(_layout_param->isWrapContent())
            doWrapContentLayout();

        _layout->begin(_layout_param);

        for(const sp<Placement>& i: _placments)
            i->doPlace(_layout_param->contentHeight(), _layout);

        const Rect p = _layout->end();
        for(const sp<Placement>& i : _placments)
            i->doLayoutEnd(p);
    }
    else
        for(const sp<Placement>& i : _placments)
            i->updateLayout();
}

void ViewGroup::doWrapContentLayout()
{
    LayoutParam lp(*_layout_param);
    Rect clientRect;
    _layout->begin(lp);
    for(const sp<Placement>& i: _placments)
        i->doWrapContentPlace(_layout, clientRect);

    _layout->end();
    _layout_param->setContentWidth(clientRect.width());
    _layout_param->setContentHeight(clientRect.height());
}

bool ViewGroup::isLayoutNeeded()
{
    bool layoutNeeded = false;
    for(auto iter = _placments.begin(); iter != _placments.end(); ++iter)
    {
        const sp<Placement>& i = *iter;
        if(i->isExpired())
        {
            iter = _placments.erase(iter);
            if(iter == _placments.end())
                return true;
            continue;
        }
        layoutNeeded = layoutNeeded || i->layoutRequested();
    }
    return layoutNeeded;
}

ViewGroup::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _parent(parent), _manifest(manifest),
      _layout(parent.getBuilder<Layout>(manifest, Constants::Attributes::LAYOUT)),
      _background(parent.getBuilder<Renderer>(manifest, Constants::Attributes::BACKGROUND)),
      _layout_param(parent.ensureBuilder<LayoutParam>(manifest))
{
}

sp<Renderer> ViewGroup::BUILDER::build(const sp<Scope>& args)
{
    const sp<ViewGroup> viewGroup = sp<ViewGroup>::make(_background->build(args), _layout->build(args), _layout_param->build(args));
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::RENDER_LAYER)
            viewGroup->addRenderer(_parent.ensure<RenderLayer>(i, args));
        else
            viewGroup->addRenderer(_parent.ensure<Renderer>(i, args));
    }
    return viewGroup;
}

}
