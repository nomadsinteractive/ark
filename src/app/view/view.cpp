#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/inf/runnable.h"
#include "core/util/conversions.h"
#include "core/util/dictionaries.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"

#include "graphics/base/bounds.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/impl/renderer/renderer_by_render_object.h"
#include "graphics/util/vec4_util.h"

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
    if(str == "actived")
        return View::STATE_ACTIVED;
    return View::STATE_DEFAULT;
}

View::View(const sp<LayoutParam>& layoutParam)
    : _layout_param(layoutParam), _state(sp<State>::make(STATE_DEFAULT))
{
}

View::View(const sp<Size>& size)
    : View(sp<LayoutParam>::make(size))
{
}

const sp<Size>& View::size()
{
    return _layout_param->size();
}

void View::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_on_enter, visitor);
    HolderUtil::visit(_on_leave, visitor);
    HolderUtil::visit(_on_push, visitor);
    HolderUtil::visit(_on_click, visitor);
    HolderUtil::visit(_on_release, visitor);
    HolderUtil::visit(_on_move, visitor);
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

void View::addState(View::State state)
{
    *_state = static_cast<State>(*_state | state);
}

void View::removeState(View::State state)
{
    *_state = static_cast<State>(*_state & ~(state));
}

const sp<Runnable>& View::onEnter() const
{
    return _on_enter;
}

bool View::fireOnEnter()
{
    if(_on_enter)
        _on_enter->run();
    addState(STATE_MOVING);
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
    removeState(STATE_MOVING);
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
    addState(STATE_PUSHING);
    return _on_push || _on_click;
}

bool View::fireOnRelease()
{
    if(_on_release)
        _on_release->run();
    removeState(STATE_PUSHING);
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
    const Event::Action action = event.action();
    if(ptin && !(*_state & View::STATE_MOVING) && (action == Event::ACTION_MOVE || action == Event::ACTION_DOWN))
        fireOnEnter();

    if(!ptin)
    {
        if(action == Event::ACTION_MOVE && (*_state & View::STATE_MOVING))
            fireOnLeave();
        if(action == Event::ACTION_UP && (*_state & View::STATE_PUSHING))
            fireOnRelease();
    }
    else if(action == Event::ACTION_UP)
    {
        if((*_state & View::STATE_PUSHING) && !fireOnRelease() && fireOnClick())
            return true;
    }
    else if(action == Event::ACTION_DOWN)
    {
        if(ptin && fireOnPush())
            return true;
    }
    else if(action == Event::ACTION_MOVE && fireOnMove(event))
        return true;

    return _layout_param->stopPropagation() && _layout_param->stopPropagation()->val();
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

View::STYLE_DISPLAY::STYLE_DISPLAY(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _display(factory, style) {
}

sp<Renderer> View::STYLE_DISPLAY::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setDisplay(_display.build(args));
    return renderer;
}

View::STYLE_GRAVITY::STYLE_GRAVITY(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _gravity(factory, style) {
}

sp<Renderer> View::STYLE_GRAVITY::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setGravity(_gravity.build(args));
    return renderer;
}

View::STYLE_MARGINS::STYLE_MARGINS(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margins(beanFactory.ensureBuilder<Vec4>(style))
{
}

sp<Renderer> View::STYLE_MARGINS::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    const sp<View> view = bindView(renderer);
    if(view)
        view->layoutParam()->setMargins(_margins->build(args));
    return renderer;
}

View::STYLE_MARGIN_TOP::STYLE_MARGIN_TOP(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_top(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_TOP::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Util::create(_margin_top->build(args), nullptr, nullptr, nullptr));
    return renderer;
}

View::STYLE_MARGIN_LEFT::STYLE_MARGIN_LEFT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_left(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_LEFT::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Util::create(nullptr, nullptr, nullptr, _margin_left->build(args)));
    return renderer;
}

View::STYLE_MARGIN_RIGHT::STYLE_MARGIN_RIGHT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_right(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_RIGHT::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Util::create(nullptr, _margin_right->build(args), nullptr, nullptr));
    return renderer;
}

View::STYLE_MARGIN_BOTTOM::STYLE_MARGIN_BOTTOM(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_bottom(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_BOTTOM::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Util::create(nullptr, nullptr, _margin_bottom->build(args), nullptr));
    return renderer;
}

View::STYLE_SIZE::STYLE_SIZE(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _size(beanFactory.ensureBuilder<Size>(style))
{
}

sp<Renderer> View::STYLE_SIZE::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setSize(_size->build(args));
    return renderer;
}

View::STYLE_ON_ENTER::STYLE_ON_ENTER(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_enter(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::STYLE_ON_ENTER::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnEnter(_on_enter->build(args));
    return renderer;
}

View::STYLE_ON_LEAVE::STYLE_ON_LEAVE(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _on_leave(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::STYLE_ON_LEAVE::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnLeave(_on_leave->build(args));
    return renderer;
}

View::STYLE_ON_PUSH::STYLE_ON_PUSH(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_push(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::STYLE_ON_PUSH::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnPush(_on_push->build(args));
    return renderer;
}

View::STYLE_ON_CLICK::STYLE_ON_CLICK(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_click(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::STYLE_ON_CLICK::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnClick(_on_click->build(args));
    return renderer;
}

View::STYLE_ON_RELEASE::STYLE_ON_RELEASE(BeanFactory& beanFactory, const sp<Builder<Renderer> >& delegate, const String& style)
    : _delegate(delegate), _on_release(beanFactory.ensureBuilder<Runnable>(style))
{
}

sp<Renderer> View::STYLE_ON_RELEASE::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnRelease(_on_release->build(args));
    return renderer;
}

View::STYLE_ON_MOVE::STYLE_ON_MOVE(BeanFactory &beanFactory, const sp<Builder<Renderer> > &delegate, const String &style)
    : _delegate(delegate), _on_move(beanFactory.ensureBuilder<EventListener>(style))
{
}

sp<Renderer> View::STYLE_ON_MOVE::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setOnMove(_on_move->build(args));
    return renderer;
}

View::STOP_PROPAGATION_STYLE::STOP_PROPAGATION_STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _stop_propagation(style ? factory.ensureBuilder<Boolean>(style) : nullptr)
{
}

sp<Renderer> View::STOP_PROPAGATION_STYLE::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setStopPropagation(_stop_propagation ? _stop_propagation->build(args) : sp<Boolean>::make<Boolean::Const>(true));
    return renderer;
}

View::STYLE_LAYOUT_PARAM::STYLE_LAYOUT_PARAM(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _layout_param(beanFactory.ensureBuilder<LayoutParam>(style))
{
}

sp<Renderer> View::STYLE_LAYOUT_PARAM::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->setLayoutParam(_layout_param->build(args));
    return renderer;
}

}
