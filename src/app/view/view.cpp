#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/impl/updatable/updatable_once_per_frame.h"
#include "core/util/boolean_type.h"
#include "core/util/math.h"
#include "core/util/updatable_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_object_with_layer.h"
#include "graphics/base/size.h"
#include "graphics/base/text.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/traits/layout_param.h"
#include "app/traits/shape.h"
#include "app/view/view_hierarchy.h"

namespace ark {

namespace {

template<size_t IDX> class LayoutSize : public Numeric {
public:
    LayoutSize(sp<View::Stub> stub)
        : _stub(std::move(stub)) {
    }

    bool update(uint64_t timestamp) override {
        return _stub->_layout_node->size().update(timestamp);
    }

    float val() override {
        const V2& size = _stub->_layout_node->size();
        return size[IDX];
    }

private:
    sp<View::Stub> _stub;
};

}

static V2 toViewportPosition(const V2& position)
{
    return Ark::instance().applicationContext()->toViewportPosition(position);
}

static V2 toPivotPosition(const sp<Boundaries>& occupies, const V2& size)
{
    const RenderEngine& renderEngine = *Ark::instance().applicationContext()->renderEngine();
    if(!occupies)
        return renderEngine.isLHS() ? V2(0, 0) : V2(0, size.y());

    const V3& occupyAABBMin = occupies->aabbMin()->val();
    const V3& occupyAABBMax = occupies->aabbMax()->val();
    return size * V2(Math::lerp(0, size.x(), occupyAABBMin.x(), occupyAABBMax.x(), 0), Math::lerp(0, size.y(), occupyAABBMin.y(), occupyAABBMax.y(), 0));
}

View::View(const sp<LayoutParam>& layoutParam, sp<RenderObjectWithLayer> background, sp<Text> text, sp<Layout> layout, sp<Boolean> visible, sp<Boolean> disposed)
    : _stub(sp<Stub>::make(layoutParam, layout ? sp<ViewHierarchy>::make(std::move(layout)) : nullptr, std::move(visible), std::move(disposed))),
      _background(std::move(background)), _text(std::move(text)), _is_discarded(sp<IsDiscarded>::make(_stub)), _is_stub_dirty(sp<UpdatableOncePerFrame>::make(_stub)),
      _is_layout_dirty(sp<UpdatableOncePerFrame>::make(sp<UpdatableIsolatedLayout>::make(_stub)))
{
    if(_background)
        addRenderObjectWithLayer(_background, true);

    _stub->_layout_node->setSize(V2(layoutParam->contentWidth(), layoutParam->contentHeight()));

    // if(_text)
    // {
    //     _text->setPosition(sp<LayoutPosition>::make(_stub, _is_layout_dirty, false, false));
    //     if(_stub->_layout_param->flexWrap() == LayoutParam::FLEX_WRAP_WRAP)
    //         _text->setLayoutSize(sp<Size>::make(sp<LayoutSize<0>>::make(_stub), sp<LayoutSize<1>>::make(_stub)));
    //     else
    //         updateTextLayout(0);
    //     _text->show(_is_discarded);
    // }
}

View::~View()
{
    _stub->dispose();
    LOGD("");
}

TypeId View::onWire(WiringContext& context)
{
    sp<Vec3> size = Vec3Type::create(sp<LayoutSize<0>>::make(_stub), sp<LayoutSize<1>>::make(_stub), nullptr);
    if(const sp<Shape>& shape = context.getComponent<Shape>())
        shape->setSize(size);
    else
        context.addComponentBuilder(make_lazy_builder<Shape>(Shape::SHAPE_ID_AABB, sp<Vec3>(size)));

    context.addComponentBuilder(make_lazy_builder<Boundaries>(std::move(size)));
    context.addComponentBuilder(make_lazy_builder<Vec3, LayoutPosition>(_stub, _is_layout_dirty, true, true));
    return Type<View>::id();
}

void View::addRenderObjectWithLayer(sp<RenderObjectWithLayer> ro, bool isBackground)
{
    ro->layerContext()->add(sp<RenderableView>::make(_stub, ro->renderObject(), ro->layerContext()->modelLoader(), isBackground), _is_layout_dirty, _is_discarded);
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
        if(layoutParam.height()._type == LayoutParam::LENGTH_TYPE_AUTO)
        {
            layoutParam.setHeightType(LayoutParam::LENGTH_TYPE_PIXEL);
            layoutParam.setHeight(size.height());
        }
    }
}

const sp<Layout::Node>& View::layoutNode() const
{
    return _stub->_layout_node;
}

const SafeVar<Boolean>& View::visible() const
{
    return _stub->_visible;
}

void View::setVisbile(sp<Boolean> visible)
{
    _stub->_visible.reset(std::move(visible));
}

const SafeVar<Boolean>& View::discarded() const
{
    return _stub->_discarded;
}

void View::setDiscarded(sp<Boolean> discarded)
{
    _stub->_discarded.reset(std::move(discarded));
}

const sp<LayoutParam>& View::layoutParam() const
{
    return _stub->_layout_param;
}

void View::setLayoutParam(sp<LayoutParam> layoutParam)
{
    _stub->_layout_param = std::move(layoutParam);
}

void View::addView(sp<View> view, sp<Boolean> discarded)
{
    if(discarded)
        view->setDiscarded(std::move(discarded));

    view->setParent(*this);
    _stub->ensureViewHierarchy().addView(std::move(view));
}

void View::setParent(const View& view)
{
    _stub->_parent_stub = view._stub;
}

void View::markAsTopView()
{
    _stub->_top_view = true;
    _stub->_parent_stub.reset(nullptr);
}

View::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISPOSED)), _visible(factory.getBuilder<Boolean>(manifest, constants::VISIBLE)),
      _layout(factory.getBuilder<Layout>(manifest, "layout")), _background(factory.getBuilder<RenderObjectWithLayer>(manifest, constants::BACKGROUND)), _text(factory.getBuilder<Text>(manifest, constants::TEXT)),
      _layout_param(factory.ensureConcreteClassBuilder<LayoutParam>(manifest, "layout-param"))
{
}

sp<View> View::BUILDER::build(const Scope& args)
{
    sp<View> view = sp<View>::make(_layout_param->build(args), _background->build(args), _text->build(args), _layout->build(args), _visible->build(args), _discarded->build(args));
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == constants::VIEW)
            view->addView(_factory.ensure<View>(i, args));
        else if(name == constants::RENDER_OBJECT)
            view->addRenderObjectWithLayer(_factory.ensure<RenderObjectWithLayer>(i, args), false);
        else
            CHECK_WARN(name == constants::TEXT || name == constants::BACKGROUND, "Ignoring unknown view child: %s", Documents::toString(i).c_str());
    }
    return view;
}

View::Stub::Stub()
    : _layout_param(sp<LayoutParam>::make(LayoutParam::Length(), LayoutParam::Length())), _layout_node(sp<Layout::Node>::make(_layout_param, nullptr)), _visible(nullptr, false),
      _discarded(nullptr, false), _top_view(true)
{
}

View::Stub::Stub(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, sp<Boolean> visible, sp<Boolean> discarded)
    : _layout_param(std::move(layoutParam)), _layout_node(sp<Layout::Node>::make(_layout_param, std::move(viewHierarchy))), _visible(std::move(visible), true),
      _discarded(std::move(discarded), false), _top_view(false)
{
}

bool View::Stub::update(uint64_t timestamp)
{
    bool dirty = UpdatableUtil::update(timestamp, _layout_param, _discarded, _visible);
    if(viewHierarchy())
        dirty = viewHierarchy()->updateLayout(_layout_node, timestamp, dirty);
    return dirty;
}

void View::Stub::dispose()
{
    _discarded.reset(sp<Boolean>::make<Boolean::Const>(true));
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
    return _discarded.val() || (parentStub ? parentStub->isDisposed() : !_top_view);
}

V3 View::Stub::getTopViewOffsetPosition(bool includePaddings) const
{
    const Layout::Node& layoutNode = _layout_node;
    const V3 layoutOffset(layoutNode.offsetPosition(), 0);
    const sp<Stub> parentStub = _parent_stub.lock();
    V3 offset = parentStub ? parentStub->getTopViewOffsetPosition(false) + layoutOffset : layoutOffset;
    if(includePaddings)
        offset += V3(layoutNode.paddings().w(), layoutNode.paddings().x(), 0);
    return _layout_param->position().val() + offset;
}

sp<Layout::Node> View::Stub::getTopViewLayoutNode() const
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
        _layout_node->_view_hierarchy = sp<ViewHierarchy>::make(nullptr);

    return _layout_node->_view_hierarchy;
}

View::RenderableView::RenderableView(sp<Stub> viewStub, sp<Renderable> renderable, sp<ModelLoader> modelLoader, bool isBackground)
    : _view_stub(viewStub), _renderable(std::move(renderable)), _model_loader(std::move(modelLoader)), _is_background(isBackground)
{
}

Renderable::StateBits View::RenderableView::updateState(const RenderRequest& renderRequest)
{
    Renderable::State state = _renderable->updateState(renderRequest);
    if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
        state.setState(Renderable::RENDERABLE_STATE_VISIBLE, _view_stub->isVisible());
    return state.stateBits();
}

Renderable::Snapshot View::RenderableView::snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state)
{
    if(const sp<Layout::Node> topViewLayoutNode = _view_stub->getTopViewLayoutNode())
    {
        const Layout::Node& layoutNode = _view_stub->_layout_node;
        const V4& paddings = layoutNode.paddings();
        const V2& layoutSize = layoutNode.size();
        const V2 size = _is_background ? layoutSize : layoutSize - V2(paddings.y() + paddings.w(), paddings.x() + paddings.z());
        const V3 topViewOffset = _view_stub->getTopViewOffsetPosition(false);
        const V3 offset = _is_background ? topViewOffset : topViewOffset + V3(paddings.w(), paddings.x(), 0);
        Renderable::Snapshot snapshot = _renderable->snapshot(pipelineInput, renderRequest, postTranslate, state);
        if(!snapshot._model)
            snapshot._model = _model_loader->loadModel(snapshot._type);
        snapshot._position += V3(toViewportPosition(toPivotPosition(snapshot._model->occupy(), size) + V2(offset.x(), offset.y())), offset.z());
        snapshot._size = V3(size, 0);
        return snapshot;
    }
    return Renderable::Snapshot(Renderable::RENDERABLE_STATE_NONE);
}

View::IsDiscarded::IsDiscarded(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

bool View::IsDiscarded::update(uint64_t timestamp)
{
    bool dirty = false;
    sp<Stub> stub = _stub;
    while(stub)
    {
        dirty = stub->_discarded.update(timestamp) || dirty;
        stub = stub->_parent_stub.lock();
    }
    return dirty;
}

bool View::IsDiscarded::val()
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
    const Layout::Node& layoutNode = _stub->_layout_node;
    const V2& size = layoutNode.size();
    const V4& paddings = layoutNode.paddings();
    const V3 offsetPosition = _stub->getTopViewOffsetPosition(false);
    float yOffset = Ark::instance().applicationContext()->renderEngine()->isLHS() ? size.y() : 0;
    float x = offsetPosition.x() + (_is_background ? 0 : paddings.w()) + (_is_center ? size.x() / 2 : 0);
    float y = (offsetPosition.y() + (_is_background ? 0 : paddings.x())) + (_is_center ? size.y() / 2 : yOffset);
    return {toViewportPosition(V2(x, y)), offsetPosition.z()};
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
