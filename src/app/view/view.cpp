#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/inf/runnable.h"
#include "core/impl/updatable/updatable_once_per_frame.h"
#include "core/types/safe_var.h"
#include "core/util/math.h"
#include "core/util/string_convert.h"

#include "graphics/base/bounds.h"
#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_object_with_layer.h"
#include "graphics/base/size.h"
#include "graphics/base/text.h"
#include "graphics/util/vec4_type.h"
#include "graphics/util/renderer_type.h"

#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"

#include "app/base/event.h"
#include "app/view/layout_param.h"
#include "app/view/view_hierarchy.h"
#include "app/inf/event_listener.h"

namespace ark {

namespace {

template<size_t IDX> class LayoutSize : public Numeric {
public:
    LayoutSize(sp<View::Stub> stub, sp<Updatable> isDirty)
        : _stub(std::move(stub)), _is_dirty(std::move(isDirty)) {
    }

    virtual bool update(uint64_t timestamp) override {
        return _is_dirty->update(timestamp);
    }

    virtual float val() override {
        return _stub->_layout_node->_size[IDX];
    }

private:
    sp<View::Stub> _stub;
    sp<Updatable> _is_dirty;

};

}

template<> ARK_API View::State StringConvert::to<String, View::State>(const String& str)
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

static V2 toViewSpacePosition(const V2& position)
{
    return Ark::instance().applicationContext()->renderEngine()->toViewportPosition(position, Ark::COORDINATE_SYSTEM_LHS);
}

static V2 toPivotPosition(const sp<Metrics>& occupies, const V2& size)
{
    if(!occupies)
        return V2(0, size.y());

    return size * V2(Math::lerp(0, size.x(), occupies->aabbMin().x(), occupies->aabbMax().x(), 0), Math::lerp(0, size.y(), occupies->aabbMin().y(), occupies->aabbMax().y(), 0));
}

View::View(const sp<LayoutParam>& layoutParam, sp<RenderObjectWithLayer> background, sp<Text> text, sp<Layout> layout, sp<LayoutV3> layoutV3, sp<Boolean> visible, sp<Boolean> disposed)
    : _stub(sp<Stub>::make(layoutParam, (layout || layoutV3) ? sp<ViewHierarchy>::make(std::move(layout), std::move(layoutV3)) : nullptr, std::move(visible), std::move(disposed))),
      _background(std::move(background)), _text(std::move(text)), _state(sp<State>::make(STATE_DEFAULT)), _is_disposed(sp<IsDisposed>::make(_stub)), _is_dirty(sp<UpdatableOncePerFrame>::make(sp<UpdatableLayoutView>::make(_stub))),
      _size(sp<Size>::make(sp<LayoutSize<0>>::make(_stub, _is_dirty), sp<LayoutSize<1>>::make(_stub, _is_dirty))), _position(sp<LayoutPosition>::make(_stub, _is_dirty, true, true))
{
    if(_background)
        addRenderObjectWithLayer(_background, true);

    if(_text)
    {
        updateTextLayout(0);
        _text->setPosition(sp<LayoutPosition>::make(_stub, _is_dirty, false, false));
        _text->show(_is_disposed);
    }

    _stub->_layout_node->_size = V2(layoutParam->contentWidth(), layoutParam->contentHeight());
}

View::View(sp<Size> size)
    : View(sp<LayoutParam>::make(std::move(size)))
{
}

View::~View()
{
    _stub->dispose();
    LOGD("");
}

const sp<Size>& View::size()
{
    return _size;
}

void View::traverse(const Holder::Visitor& visitor)
{
//    HolderUtil::visit(_on_enter, visitor);
//    HolderUtil::visit(_on_leave, visitor);
//    HolderUtil::visit(_on_push, visitor);
//    HolderUtil::visit(_on_click, visitor);
//    HolderUtil::visit(_on_release, visitor);
//    HolderUtil::visit(_on_move, visitor);
//    HolderUtil::visit(_background, visitor);

    if(_stub->viewHierarchy())
        _stub->viewHierarchy()->traverse(visitor);
}

void View::render(RenderRequest& renderRequest, const V3& position)
{
    updateTextLayout(renderRequest.timestamp());

    if(_stub->viewHierarchy())
    {
        _stub->viewHierarchy()->updateLayout(*this, renderRequest.timestamp(), _is_dirty->update(renderRequest.timestamp()));
        _stub->viewHierarchy()->render(renderRequest, position);
    }
}

void View::addRenderer(const sp<Renderer>& renderer)
{
    _stub->ensureViewHierarchy().addRenderer(renderer);
}

bool View::onEvent(const Event& event, float x, float y, bool ptin)
{
    return (_stub->viewHierarchy() ? _stub->viewHierarchy()->onEvent(event, x, y) : false) || dispatchEvent(event, ptin);
}

void View::addRenderObjectWithLayer(sp<RenderObjectWithLayer> ro, bool isBackground)
{
    ro->layerContext()->add(sp<RenderableViewSlot>::make(_stub, ro->renderObject(), ro->layerContext()->modelLoader(), isBackground), _is_dirty, _is_disposed);
}

const sp<Vec3>& View::position() const
{
    return _position;
}

void View::updateLayout()
{
    if(_stub->viewHierarchy())
        _stub->viewHierarchy()->updateLayout(*this, 0, false);
}

void View::updateTextLayout(uint64_t timestamp)
{
    if(_text && (!timestamp || _text->update(timestamp)))
    {
        LayoutParam& layoutParam = _stub->_layout_param;
        Size& size = _text->size();
        if(layoutParam.widthType() == LayoutParam::LENGTH_TYPE_AUTO)
        {
            layoutParam.setWidthType(LayoutParam::LENGTH_TYPE_PIXEL);
            layoutParam.setWidth(size.width());
        }
        if(layoutParam.heightType() == LayoutParam::LENGTH_TYPE_AUTO)
        {
            layoutParam.setHeightType(LayoutParam::LENGTH_TYPE_PIXEL);
            layoutParam.setHeight(size.height());
        }
    }
}

const sp<LayoutV3::Node>& View::layoutNode() const
{
    return _stub->_layout_node;
}

const sp<Boolean>& View::visible() const
{
    return _stub->_visible.ensure();
}

void View::setVisbile(sp<Boolean> visible)
{
    _stub->_visible.reset(std::move(visible));
}

sp<Boolean> View::disposed() const
{
    return _stub->_disposed;
}

void View::setDisposed(sp<Boolean> disposed)
{
    _stub->_disposed->reset(std::move(disposed));
}

const sp<LayoutParam>& View::layoutParam() const
{
    return _stub->_layout_param;
}

void View::setLayoutParam(sp<LayoutParam> layoutParam)
{
    _stub->_layout_param = std::move(layoutParam);
}

void View::addView(sp<View> view, sp<Boolean> disposable)
{
    if(disposable)
        view->setDisposed(std::move(disposable));

    view->setParent(*this);
    _stub->ensureViewHierarchy().addView(std::move(view));
}

void View::setParent(const View& view)
{
    _stub->_parent_stub = view._stub;
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

void View::markAsTopView()
{
    _stub->_top_view = true;
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

    return ptin && _stub->_layout_param->stopPropagation() && _stub->_layout_param->stopPropagation()->val();
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
    bindView(renderer)->layoutParam()->setMargins(Vec4Type::create(_margin_top->build(args), nullptr, nullptr, nullptr));
    return renderer;
}

View::STYLE_MARGIN_LEFT::STYLE_MARGIN_LEFT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_left(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_LEFT::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Type::create(nullptr, nullptr, nullptr, _margin_left->build(args)));
    return renderer;
}

View::STYLE_MARGIN_RIGHT::STYLE_MARGIN_RIGHT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_right(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_RIGHT::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Type::create(nullptr, _margin_right->build(args), nullptr, nullptr));
    return renderer;
}

View::STYLE_MARGIN_BOTTOM::STYLE_MARGIN_BOTTOM(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _margin_bottom(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_MARGIN_BOTTOM::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setMargins(Vec4Type::create(nullptr, nullptr, _margin_bottom->build(args), nullptr));
    return renderer;
}

View::STYLE_LAYOUT_WEIGHT::STYLE_LAYOUT_WEIGHT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style)
    : _delegate(delegate), _layout_weight(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Renderer> View::STYLE_LAYOUT_WEIGHT::build(const Scope& args)
{
    sp<Renderer> renderer = _delegate->build(args);
    bindView(renderer)->layoutParam()->setFlexGrow(_layout_weight->build(args)->val());
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

View::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _disposed(factory.getBuilder<Boolean>(manifest, Constants::Attributes::DISPOSED)), _visible(factory.getBuilder<Boolean>(manifest, Constants::Attributes::VISIBLE)),
      _layout(factory.getBuilder<Layout>(manifest, Constants::Attributes::LAYOUT)), _layout_v3(factory.getBuilder<LayoutV3>(manifest, "layout-v3")),
      _background(factory.getBuilder<RenderObjectWithLayer>(manifest, Constants::Attributes::BACKGROUND)), _text(factory.getBuilder<Text>(manifest, Constants::Attributes::TEXT)),
      _layout_param(factory.ensureConcreteClassBuilder<LayoutParam>(manifest, "layout-param"))
{
}

sp<View> View::BUILDER::build(const Scope& args)
{
    sp<View> view = sp<View>::make(_layout_param->build(args), _background->build(args), _text->build(args), _layout->build(args), _layout_v3->build(args), _visible->build(args), _disposed->build(args));
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::LAYER)
            view->addRenderer(_factory.ensureDecorated<Renderer, Layer>(i, args));
        else if(name == Constants::Attributes::VIEW)
            view->addView(_factory.ensure<View>(i, args));
        else if(name == Constants::Attributes::RENDER_OBJECT)
            view->addRenderObjectWithLayer(_factory.ensure<RenderObjectWithLayer>(i, args), false);
        else if(name != Constants::Attributes::BACKGROUND && name != Constants::Attributes::TEXT)
            view->addRenderer(_factory.ensure<Renderer>(i, args));
    }
    return view;
}

View::BUILDER_VIEW::BUILDER_VIEW(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest)
{
}

sp<Renderer> View::BUILDER_VIEW::build(const Scope& args)
{
    return _impl.build(args);
}

View::Stub::Stub(const sp<LayoutParam>& layoutParam, sp<ViewHierarchy> viewHierarchy, sp<Boolean> visible, sp<Boolean> disposed)
    : _layout_param(Null::toSafePtr<LayoutParam>(layoutParam)), _layout_node(sp<LayoutV3::Node>::make(_layout_param, std::move(viewHierarchy))), _visible(std::move(visible), true),
      _disposed(sp<BooleanWrapper>::make(std::move(disposed))), _top_view(false)
{
}

bool View::Stub::update(uint64_t timestamp)
{
    bool dirty = UpdatableUtil::update(timestamp, _layout_param, _disposed, _visible);
    if(viewHierarchy())
        return viewHierarchy()->update(timestamp) || dirty;
    return dirty;
}

void View::Stub::dispose()
{
    _disposed->set(true);
    _layout_param = nullptr;
    _layout_node = nullptr;
    _parent_stub = nullptr;
}

bool View::Stub::isVisible() const
{
    return _visible.val() && (_parent_stub ? _parent_stub->isVisible() : _top_view);
}

bool View::Stub::isDisposed() const
{
    return _disposed->val() || (_parent_stub ? _parent_stub->isDisposed() : false);
}

V3 View::Stub::getTopViewOffsetPosition() const
{
    const LayoutV3::Node& layoutNode = _layout_node;
    const V3 layoutOffset = V3(layoutNode._offset_position, 0);
    const V3 offset = (_parent_stub ? _parent_stub->getTopViewOffsetPosition() + layoutOffset : layoutOffset) + V3(layoutNode._paddings.w(), layoutNode._paddings.x(), 0);
    return _layout_param->position().val() + offset;
}

sp<LayoutV3::Node> View::Stub::getTopViewLayoutNode() const
{
    if(_top_view)
        return _layout_node;
    return _parent_stub ? _parent_stub->getTopViewLayoutNode() : nullptr;
}

const sp<ViewHierarchy>& View::Stub::viewHierarchy() const
{
    return _layout_node->_view_hierarchy;
}

ViewHierarchy& View::Stub::ensureViewHierarchy()
{
    if(!_layout_node->_view_hierarchy)
        _layout_node->_view_hierarchy = sp<ViewHierarchy>::make(nullptr, nullptr);

    return _layout_node->_view_hierarchy;
}

View::RenderableViewSlot::RenderableViewSlot(sp<Stub> viewStub, sp<Renderable> renderable, sp<ModelLoader> modelLoader, bool isBackground)
    : _view_stub(viewStub), _renderable(std::move(renderable)), _model_loader(std::move(modelLoader)), _is_background(isBackground)
{
}

Renderable::StateBits View::RenderableViewSlot::updateState(const RenderRequest& renderRequest)
{
    Renderable::State state = _renderable->updateState(renderRequest);
    if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
        state.setState(Renderable::RENDERABLE_STATE_VISIBLE, _view_stub->isVisible());
    return state.stateBits();
}

Renderable::Snapshot View::RenderableViewSlot::snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state)
{
    sp<LayoutV3::Node> topViewLayoutNode = _view_stub->getTopViewLayoutNode();
    if(topViewLayoutNode)
    {
        const LayoutV3::Node& layoutNode = _view_stub->_layout_node;
        const V4& paddings = layoutNode._paddings;
        const V2 size = _is_background ? layoutNode._size : layoutNode._size - V2(paddings.y() + paddings.w(), paddings.x() + paddings.z());
        const V3 topViewOffset = _view_stub->getTopViewOffsetPosition();
        const V3 offset = _is_background ? topViewOffset : topViewOffset + V3(paddings.w(), paddings.x(), 0);
        Renderable::Snapshot snapshot = _renderable->snapshot(pipelineInput, renderRequest, postTranslate, state);
        if(!snapshot._model)
            snapshot._model = _model_loader->loadModel(snapshot._type);
        snapshot._position += V3(toViewSpacePosition(toPivotPosition(snapshot._model->occupies(), size) + V2(offset.x(), offset.y())), offset.z());
        snapshot._size = V3(size, 0);
        return snapshot;
    }
    return Renderable::Snapshot(Renderable::RENDERABLE_STATE_NONE);
}

View::IsDisposed::IsDisposed(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

bool View::IsDisposed::update(uint64_t timestamp)
{
    bool dirty = false;
    Stub* stub = _stub.get();
    while(stub)
    {
        dirty = stub->_disposed->update(timestamp) || dirty;
        stub = stub->_parent_stub.get();
    }
    return dirty;
}

bool View::IsDisposed::val()
{
    return _stub->isDisposed();
}

View::LayoutPosition::LayoutPosition(sp<Stub> stub, sp<Updatable> updatable, bool isBackground, bool isCenter)
    : _stub(std::move(stub)), _updatable(std::move(updatable)), _is_background(isBackground), _is_center(isCenter)
{
}

bool View::LayoutPosition::update(uint64_t timestamp)
{
    return _updatable->update(timestamp);
}

V3 View::LayoutPosition::val()
{
    const LayoutV3::Node& layoutNode = _stub->_layout_node;
    const V2& size = layoutNode._size;
    const V4& paddings = layoutNode._paddings;
    const V3 offsetPosition = _stub->getTopViewOffsetPosition();
    float x = offsetPosition.x() + (_is_background ? 0 : paddings.w()) + (_is_center ? size.x() / 2 : 0);
    float y = (offsetPosition.y() + (_is_background ? 0 : paddings.x())) + (_is_center ? size.y() / 2 : size.y());
    return V3(toViewSpacePosition(V2(x, y)), offsetPosition.z());
}

View::UpdatableLayoutView::UpdatableLayoutView(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

bool View::UpdatableLayoutView::update(uint64_t timestamp)
{
    Stub* stub = _stub.get();
    while(stub)
    {
        const sp<ViewHierarchy>& viewHierarchy = stub->viewHierarchy();
        if(viewHierarchy && viewHierarchy->layout())
            break;

        stub = stub->_parent_stub.get();
    }

    return stub ? stub->update(timestamp) : false;
}

}
