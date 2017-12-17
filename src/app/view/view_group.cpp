#include "app/view/view_group.h"

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/util/log.h"

#include "graphics/base/frame.h"
#include "graphics/base/render_layer.h"
#include "graphics/impl/renderer/renderer_delegate.h"

#include "app/base/event.h"
#include "app/inf/layout.h"

namespace ark {

ViewGroup::Placement::Placement(const sp<Renderer>& renderer)
    : _x(0), _y(0), _renderer(renderer), _view(renderer.as<View>()), _view_group(renderer.as<ViewGroup>()), _renderer_delegate(renderer.as<RendererDelegate>()), _expirable(renderer.as<Expired>())
{
    NOT_NULL(renderer);
}

const sp<Renderer>& ViewGroup::Placement::renderer() const
{
    return _renderer;
}

const sp<ViewGroup>& ViewGroup::Placement::viewGroup() const
{
    return _view_group;
}

const sp<View>& ViewGroup::Placement::view() const
{
    return _view;
}

bool ViewGroup::Placement::isExpired() const
{
    return _expirable && _expirable->val();
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

void ViewGroup::Placement::doEnd(const Rect& p)
{
    if(_view)
    {
        _x += p.left();
        if(g_isOriginBottom)
            _y -= p.top();
        else
            _y += p.top();
    }
}

void ViewGroup::Placement::render(RenderRequest& renderRequest, float x, float y) const
{
    _renderer->render(renderRequest, x + _x, y + _y);
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
    : View(!layoutParam && background ? sp<LayoutParam>::make(background.size()) : layoutParam), _background(background.renderer()), _layout(layout), _layout_requested(false)
{
    DCHECK(!_layout || _layout_param, "Null LayoutParam. This would happen if your ViewGroup has neither background or size defined.");
}

ViewGroup::~ViewGroup()
{
    LOGD("");
}

void ViewGroup::addRenderer(const sp<Renderer>& renderer)
{
    NOT_NULL(renderer);
    _renderers.push_back(renderer);
    _layout_requested = true;
}

void ViewGroup::render(RenderRequest& renderRequest, float x, float y)
{
    if(_background)
        _background->render(renderRequest, x, y);

    if(_layout_requested)
        doLayout();

    for(const sp<Placement>& i: _placments)
    {
        i->render(renderRequest, x, y);
        _layout_requested = _layout_requested || i->isExpired();
    }
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
    _layout_requested = false;
    _placments.clear();

    if(_layout)
        _layout->begin(_layout_param);

    for(const sp<Renderer>& i : _renderers)
    {
        const sp<Placement> placement = sp<Placement>::make(i);
        _placments.push_back(placement);
        if(_layout)
            placement->doPlace(_layout_param->contentHeight(), _layout);
    }

    if(_layout)
    {
        Rect p = _layout->end();
        for(const sp<Placement>& i : _placments)
            i->doEnd(p);
    }
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
