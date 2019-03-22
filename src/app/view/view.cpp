#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/inf/runnable.h"
#include "core/util/conversions.h"
#include "core/util/dictionaries.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/render_object.h"
#include "graphics/impl/renderer/renderer_by_render_object.h"

#include "app/base/event.h"
#include "app/view/layout_param.h"
#include "app/inf/event_listener.h"

namespace ark {

template<> ARK_API View::State Conversions::to<String, View::State>(const String& str)
{
    if(str == "default")
        return View::STATE_DEFAULT;
    if(str == "moving")
        return View::STATE_MOVING;
    if(str == "pushing")
        return View::STATE_PUSHING;
    return View::STATE_DEFAULT;
}

template<> ARK_API View::Gravity Conversions::to<String, View::Gravity>(const String& s)
{
    if(s == "none")
        return View::NONE;

    uint32_t gravity = View::NONE;
    for(const String& i : s.split('|'))
    {
        const String str = i.strip();
        if(str == "left")
            gravity |= View::LEFT;
        else if(str == "right")
            gravity |= View::RIGHT;
        else if(str == "top")
            gravity |= View::TOP;
        else if(str == "bottom")
            gravity |= View::BOTTOM;
        else if(str == "center")
            gravity |= View::CENTER;
        else if(str == "center_horizontal")
            gravity |= View::CENTER_HORIZONTAL;
        else if(str == "center_vertical")
            gravity |= View::CENTER_VERTICAL;
        else
            DFATAL("Unknown gravity value: \"%s\"", i.c_str());
    }
    return static_cast<View::Gravity>(gravity);
}

View::View(const sp<LayoutParam>& layoutParam)
    : _layout_param(layoutParam), _state(STATE_DEFAULT)
{
}

View::View(const sp<Size>& size)
    : View(sp<LayoutParam>::make(size))
{
}

const SafePtr<Size>& View::size()
{
    return _layout_param->size();
}

const SafePtr<LayoutParam>& View::layoutParam() const
{
    return _layout_param;
}

void View::setLayoutParam(const sp<LayoutParam>& layoutParam)
{
    _layout_param = layoutParam;
}

View::State View::state() const
{
    return _state;
}

const sp<Runnable>& View::onEnter() const
{
    return _on_enter;
}

bool View::fireOnEnter()
{
    if(_on_enter)
        _on_enter->run();
    _state = static_cast<State>(_state | STATE_MOVING);
    return static_cast<bool>(_on_enter);
}

void View::setOnEnter(const sp<Runnable>& onEnter)
{
    _on_enter = onEnter;
}

const sp<Runnable>& View::onLeave() const
{
    return _on_leave;
}

bool View::fireOnLeave()
{
    if(_on_leave)
        _on_leave->run();
    _state = static_cast<State>(_state & ~(STATE_MOVING));
    return static_cast<bool>(_on_leave);
}

void View::setOnLeave(const sp<Runnable>& onLeave)
{
    _on_leave = onLeave;
}

const sp<Runnable>& View::onPush() const
{
    return _on_push;
}

bool View::fireOnPush()
{
    if(_on_push)
        _on_push->run();
    _state = static_cast<State>(_state | STATE_PUSHING);
    return _on_push || _on_click;
}

bool View::fireOnRelease()
{
    if(_on_release)
        _on_release->run();
    _state = static_cast<State>(_state & ~(STATE_PUSHING));
    return false;
}

void View::setOnPush(const sp<Runnable>& onPush)
{
    _on_push = onPush;
}

const sp<Runnable>& View::onClick() const
{
    return _on_click;
}

bool View::fireOnClick()
{
    if(_on_click)
        _on_click->run();
    _state = STATE_MOVING;
    return static_cast<bool>(_on_click);
}

bool View::fireOnMove(const Event& event)
{
    return _on_move ? _on_move->onEvent(event) : false;
}

void View::setOnClick(const sp<Runnable>& onClick)
{
    _on_click = onClick;
}

const sp<Runnable>& View::onRelease() const
{
    return _on_release;
}

void View::setOnRelease(const sp<Runnable>& onRelease)
{
    _on_release = onRelease;
}

const sp<EventListener>& View::onMove() const
{
    return _on_move;
}

void View::setOnMove(const sp<EventListener>& onMove)
{
    _on_move = onMove;
}

bool View::dispatchEvent(const Event& event, bool ptin)
{
    switch(_state)
    {
    case View::STATE_DEFAULT:
        if(ptin)
        {
            if(event.action() == Event::ACTION_MOVE)
                fireOnEnter();
        }
        else
            break;
    case View::STATE_PUSHING:
    case View::STATE_MOVING:
    case View::STATE_MOVING_PUSHING:
        if(!ptin)
        {
            if(event.action() == Event::ACTION_MOVE)
                fireOnLeave();
        }
        else if(event.action() == Event::ACTION_UP && !fireOnRelease() && fireOnClick())
            return true;
        else if(event.action() == Event::ACTION_DOWN && fireOnPush())
            return true;
        break;
    }
    if(event.action() == Event::ACTION_MOVE && ptin)
        return fireOnMove(event);
    if(event.action() == Event::ACTION_UP && (_state & View::STATE_PUSHING))
        fireOnRelease();
    return false;
}

namespace {

sp<View> bindView(sp<Renderer>& decorated)
{
    const sp<View> view = decorated.as<View>();
    if(view)
        return view;

    const sp<Block> block = decorated.as<Block>();
    const sp<View> decoratedView = sp<View>::make(block ? static_cast<const sp<Size>&>(block->size()) : sp<Size>::null());
    decorated.absorb(decoratedView);
    return decoratedView;
}

}

View::DISPLAY_DECORATOR::DISPLAY_DECORATOR(BeanFactory& /*beanFactory*/, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _display(Strings::parse<LayoutParam::Display>(style)) {
}

sp<Renderer> View::DISPLAY_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setDisplay(_display);
    return renderer;
}

View::MARGINS_DECORATOR::MARGINS_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _bean_factory(beanFactory), _delegate(delegate), _margins(style)
{
}

sp<Renderer> View::MARGINS_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    const sp<View> view = bindView(renderer);
    if(view)
    {
        const Rect margins = Dictionaries::get<Rect>(_bean_factory, _margins, args);
        view->layoutParam()->margins() = Rect(margins.bottom(), margins.left(), margins.top(), margins.right());
    }
    return renderer;
}

View::MARGIN_TOP_DECORATOR::MARGIN_TOP_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _bean_factory(beanFactory), _delegate(delegate), _margin_top(style)
{
}

sp<Renderer> View::MARGIN_TOP_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->margins().setTop(Dictionaries::get<float>(_bean_factory, _margin_top, args));
    return renderer;
}

View::MARGIN_LEFT_DECORATOR::MARGIN_LEFT_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _bean_factory(beanFactory), _delegate(delegate), _margin_left(style)
{
}

sp<Renderer> View::MARGIN_LEFT_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->margins().setLeft(Dictionaries::get<float>(_bean_factory, _margin_left, args));
    return renderer;
}

View::MARGIN_RIGHT_DECORATOR::MARGIN_RIGHT_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _bean_factory(beanFactory), _delegate(delegate), _margin_right(style)
{
}

sp<Renderer> View::MARGIN_RIGHT_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->margins().setRight(Dictionaries::get<float>(_bean_factory, _margin_right, args));
    return renderer;
}

View::MARGIN_BOTTOM_DECORATOR::MARGIN_BOTTOM_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _bean_factory(beanFactory), _delegate(delegate), _margin_bottom(style)
{
}

sp<Renderer> View::MARGIN_BOTTOM_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->margins().setBottom(Dictionaries::get<float>(_bean_factory, _margin_bottom, args));
    return renderer;
}

View::SIZE_DECORATOR::SIZE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _size(beanFactory.ensureBuilder<Size>(style))
{
}

sp<Renderer> View::SIZE_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setSize(_size->build(args));
    return renderer;
}

View::ON_ENTER_DECORATOR::ON_ENTER_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_enter(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::ON_ENTER_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnEnter(_on_enter->build(args));
    return renderer;
}

View::ON_LEAVE_DECORATOR::ON_LEAVE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _on_leave(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::ON_LEAVE_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnLeave(_on_leave->build(args));
    return renderer;
}

View::ON_PUSH_DECORATOR::ON_PUSH_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_push(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::ON_PUSH_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnPush(_on_push->build(args));
    return renderer;
}

View::ON_CLICK_DECORATOR::ON_CLICK_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_click(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::ON_CLICK_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnClick(_on_click->build(args));
    return renderer;
}

View::ON_RELEASE_DECORATOR::ON_RELEASE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_release(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::ON_RELEASE_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnRelease(_on_release->build(args));
    return renderer;
}

View::ON_MOVE_DECORATOR::ON_MOVE_DECORATOR(BeanFactory &beanFactory, const sp<Builder<Renderer> > &delegate, const String &style)
    : _delegate(delegate), _on_move(beanFactory.ensureBuilder<EventListener>(style))
{
}

sp<Renderer> View::ON_MOVE_DECORATOR::build(const sp<Scope> &args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnMove(_on_move->build(args));
    return renderer;
}

View::LAYOUT_PARAM_DECORATOR::LAYOUT_PARAM_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _layout_param(beanFactory.ensureBuilder<LayoutParam>(style))
{
}

sp<Renderer> View::LAYOUT_PARAM_DECORATOR::build(const sp<Scope>& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setLayoutParam(_layout_param->build(args));
    return renderer;
}

}
