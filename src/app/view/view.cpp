#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/impl/updatable/updatable_once_per_frame.h"
#include "core/util/boolean_type.h"
#include "core/util/math.h"
#include "core/util/string_convert.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_object_with_layer.h"
#include "graphics/base/size.h"
#include "graphics/base/text.h"
#include "graphics/util/vec4_type.h"
#include "graphics/util/renderer_type.h"

#include "renderer/base/model.h"

#include "app/base/application_context.h"
#include "app/view/layout_param.h"
#include "app/view/view_hierarchy.h"

namespace ark {

namespace {

template<size_t IDX> class LayoutSize : public Numeric {
public:
    LayoutSize(sp<View::Stub> stub)
        : _stub(std::move(stub)) {
    }

    virtual bool update(uint64_t timestamp) override {
        return _stub->_layout_node->size().update(timestamp);
    }

    virtual float val() override {
        const V2& size = _stub->_layout_node->size();
        return size[IDX];
    }

private:
    sp<View::Stub> _stub;

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

static V2 toViewportPosition(const V2& position)
{
    return Ark::instance().applicationContext()->toViewportPosition(position);
}

static V2 toPivotPosition(const sp<Metrics>& occupies, const V2& size)
{
    if(!occupies)
        return V2(0, size.y());

    return size * V2(Math::lerp(0, size.x(), occupies->aabbMin().x(), occupies->aabbMax().x(), 0), Math::lerp(0, size.y(), occupies->aabbMin().y(), occupies->aabbMax().y(), 0));
}

View::View(const sp<LayoutParam>& layoutParam, sp<RenderObjectWithLayer> background, sp<Text> text, sp<Layout> layout, sp<LayoutV3> layoutV3, sp<Boolean> visible, sp<Boolean> disposed)
    : _stub(sp<Stub>::make(layoutParam, (layout || layoutV3) ? sp<ViewHierarchy>::make(std::move(layout), std::move(layoutV3)) : nullptr, std::move(visible), std::move(disposed))),
      _background(std::move(background)), _text(std::move(text)), _state(sp<State>::make(STATE_DEFAULT)), _is_disposed(sp<IsDisposed>::make(_stub)), _is_stub_dirty(sp<UpdatableOncePerFrame>::make(_stub)),
      _is_layout_dirty(sp<UpdatableOncePerFrame>::make(sp<UpdatableIsolatedLayout>::make(_stub))), _size(sp<Size>::make(sp<LayoutSize<0>>::make(_stub), sp<LayoutSize<1>>::make(_stub))),
      _position(sp<LayoutPosition>::make(_stub, _is_layout_dirty, true, true))
{
    if(_background)
        addRenderObjectWithLayer(_background, true);

    _stub->_layout_node->setSize(V2(layoutParam->contentWidth(), layoutParam->contentHeight()));

    if(_text)
    {
        _text->setPosition(sp<LayoutPosition>::make(_stub, _is_layout_dirty, false, false));
        if(_stub->_layout_param->flexWrap() == LayoutParam::FLEX_WRAP_WRAP)
            _text->setLayoutSize(_size);
        else
            updateTextLayout(0);
        _text->show(_is_disposed);
    }
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
    if(_stub->viewHierarchy())
        _stub->viewHierarchy()->traverse(visitor);
}

void View::render(RenderRequest& renderRequest, const V3& position)
{
}

void View::addRenderObjectWithLayer(sp<RenderObjectWithLayer> ro, bool isBackground)
{
    ro->layerContext()->add(sp<RenderableViewSlot>::make(_stub, ro->renderObject(), ro->layerContext()->modelLoader(), isBackground), _is_layout_dirty, _is_disposed);
}

const sp<Vec3>& View::position() const
{
    return _position;
}

bool View::updateLayout(uint64_t timestamp) const
{
    return _is_stub_dirty->update(timestamp);
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
    return _stub->_disposed.ensure();
}

void View::setDisposed(sp<Boolean> disposed)
{
    _stub->_disposed.reset(std::move(disposed));
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

void View::markAsTopView()
{
    _stub->_top_view = true;
    _stub->_parent_stub.reset(nullptr);
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
        if(name == Constants::Attributes::VIEW)
            view->addView(_factory.ensure<View>(i, args));
        else if(name == Constants::Attributes::RENDER_OBJECT)
            view->addRenderObjectWithLayer(_factory.ensure<RenderObjectWithLayer>(i, args), false);
        else
            CHECK_WARN(name == Constants::Attributes::TEXT || name == Constants::Attributes::BACKGROUND, "Ignoring unknown view child: %s", Documents::toString(i).c_str());
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

View::Stub::Stub()
    : _layout_param(sp<LayoutParam>::make(LayoutParam::Length(), LayoutParam::Length())), _layout_node(sp<LayoutV3::Node>::make(_layout_param, nullptr)), _visible(nullptr, false),
      _disposed(nullptr, false), _top_view(true)
{
}

View::Stub::Stub(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, sp<Boolean> visible, sp<Boolean> disposed)
    : _layout_param(std::move(layoutParam)), _layout_node(sp<LayoutV3::Node>::make(_layout_param, std::move(viewHierarchy))), _visible(std::move(visible), true),
      _disposed(std::move(disposed), false), _parent_stub(sp<Stub>(Global<Stub>())), _top_view(false)
{
}

bool View::Stub::update(uint64_t timestamp)
{
    bool dirty = UpdatableUtil::update(timestamp, _layout_param, _disposed, _visible);
    if(viewHierarchy())
        dirty = viewHierarchy()->updateLayout(_layout_param, _layout_node, timestamp, dirty);
    return dirty;
}

void View::Stub::dispose()
{
    _disposed.reset(BooleanType::TRUE);
    _layout_param = nullptr;
    _layout_node = nullptr;
}

bool View::Stub::isVisible() const
{
    const sp<Stub> parentStub = _parent_stub.lock();
    return _visible.val() && (_top_view || (parentStub ? parentStub->isVisible() : false));
}

bool View::Stub::isDisposed() const
{
    const sp<Stub> parentStub = _parent_stub.lock();
    return _disposed.val() || (parentStub ? parentStub->isDisposed() : !_top_view);
}

V3 View::Stub::getTopViewOffsetPosition(bool includePaddings) const
{
    const LayoutV3::Node& layoutNode = _layout_node;
    const V3 layoutOffset = V3(layoutNode.offsetPosition(), 0);
    const sp<Stub> parentStub = _parent_stub.lock();
    V3 offset = parentStub ? parentStub->getTopViewOffsetPosition(false) + layoutOffset : layoutOffset;
    if(includePaddings)
        offset += V3(layoutNode.paddings().w(), layoutNode.paddings().x(), 0);
    return _layout_param->position().val() + offset;
}

sp<LayoutV3::Node> View::Stub::getTopViewLayoutNode() const
{
    if(_top_view)
        return _layout_node;
    const sp<Stub> parentStub = _parent_stub.lock();
    return parentStub ? parentStub->getTopViewLayoutNode() : nullptr;
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
        const V4& paddings = layoutNode.paddings();
        const V2& layoutSize = layoutNode.size();
        const V2 size = _is_background ? layoutSize : layoutSize - V2(paddings.y() + paddings.w(), paddings.x() + paddings.z());
        const V3 topViewOffset = _view_stub->getTopViewOffsetPosition(false);
        const V3 offset = _is_background ? topViewOffset : topViewOffset + V3(paddings.w(), paddings.x(), 0);
        Renderable::Snapshot snapshot = _renderable->snapshot(pipelineInput, renderRequest, postTranslate, state);
        if(!snapshot._model)
            snapshot._model = _model_loader->loadModel(snapshot._type);
        snapshot._position += V3(toViewportPosition(toPivotPosition(snapshot._model->occupies(), size) + V2(offset.x(), offset.y())), offset.z());
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
    sp<Stub> stub = _stub;
    while(stub)
    {
        dirty = stub->_disposed.update(timestamp) || dirty;
        stub = stub->_parent_stub.lock();
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
    const V2& size = layoutNode.size();
    const V4& paddings = layoutNode.paddings();
    const V3 offsetPosition = _stub->getTopViewOffsetPosition(false);
    float x = offsetPosition.x() + (_is_background ? 0 : paddings.w()) + (_is_center ? size.x() / 2 : 0);
    float y = (offsetPosition.y() + (_is_background ? 0 : paddings.x())) + (_is_center ? size.y() / 2 : size.y());
    return V3(toViewportPosition(V2(x, y)), offsetPosition.z());
}

View::UpdatableIsolatedLayout::UpdatableIsolatedLayout(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

bool View::UpdatableIsolatedLayout::update(uint64_t timestamp)
{
    sp<Stub> stub = _stub;
    while(stub)
    {
        if(stub->_top_view)
            break;

        const sp<ViewHierarchy>& viewHierarchy = stub->viewHierarchy();
        if(viewHierarchy && viewHierarchy->isIsolatedLayout())
            break;

        stub = stub->_parent_stub.lock();
    }
    return stub ? stub->update(timestamp) : false;
}

}
