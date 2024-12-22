#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/updatable/updatable_once_per_frame.h"
#include "core/util/boolean_type.h"
#include "core/util/math.h"
#include "core/util/updatable_util.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_object.h"
#include "graphics/traits/layout_param.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/renderable_type.h"

#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/entity.h"
#include "app/traits/shape.h"
#include "app/traits/with_text.h"
#include "app/view/view_hierarchy.h"
#include "core/util/wirable_type.h"

namespace ark {

struct View::Stub final : Updatable {
    Stub(sp<LayoutParam> layoutParam, sp<Boolean> visible, sp<Boolean> discarded)
        : _hierarchy(layoutParam->layout() ? sp<ViewHierarchy>::make(layoutParam->layout()) : nullptr), _layout_node(sp<Layout::Node>::make(std::move(layoutParam))), _visible(std::move(visible), true), _discarded(std::move(discarded), false), _top_view(false)
    {
    }

    bool update(uint64_t timestamp) override
    {
        bool dirty = UpdatableUtil::update(timestamp, _layout_node->_layout_param, _discarded, _visible);
        if(_hierarchy)
            dirty = _hierarchy->updateLayout(_layout_node, timestamp, dirty);
        return dirty;
    }

    void dispose()
    {
        _discarded.reset(sp<Boolean>::make<Boolean::Const>(true));
        _layout_node->_layout_param = nullptr;
        _layout_node = nullptr;
    }

    bool isVisible() const
    {
        const sp<Stub> parentStub = _parent_stub.lock();
        return _visible.val() && (_top_view || (parentStub ? parentStub->isVisible() : false));
    }

    bool isDiscarded() const
    {
        const sp<Stub> parentStub = _parent_stub.lock();
        return _discarded.val() || (parentStub ? parentStub->isDiscarded() : !_top_view);
    }

    V3 getTopViewOffsetPosition(bool includePaddings) const
    {
        const Layout::Node& layoutNode = _layout_node;
        const V3 layoutOffset(layoutNode.offsetPosition(), 0);
        const sp<Stub> parentStub = _parent_stub.lock();
        V3 offset = parentStub ? parentStub->getTopViewOffsetPosition(false) + layoutOffset : layoutOffset;
        if(includePaddings)
            offset += V3(layoutNode.paddings().w(), layoutNode.paddings().x(), 0);
        return layoutNode._layout_param->position().val() + offset;
    }

    sp<Layout::Node> getTopViewLayoutNode() const
    {
        if(_top_view)
            return _layout_node;
        const sp<Stub> parentStub = _parent_stub.lock();
        return parentStub ? parentStub->getTopViewLayoutNode() : nullptr;
    }

    ViewHierarchy& ensureViewHierarchy()
    {
        if(!_hierarchy)
            _hierarchy = sp<ViewHierarchy>::make(nullptr);
        return _hierarchy;
    }

    sp<ViewHierarchy> _hierarchy;
    sp<Layout::Node> _layout_node;

    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _discarded;

    WeakPtr<Stub> _parent_stub;
    bool _top_view;
};

namespace {

V2 toViewportPosition(const V2& position)
{
    return Ark::instance().applicationContext()->toViewportPosition(position);
}

V2 toPivotPosition(const sp<Boundaries>& occupies, const V2& size)
{
    if(!occupies)
        return Ark::instance().applicationContext()->renderEngine()->isLHS() ? V2(0, 0) : V2(0, size.y());

    return occupies->toPivotPosition(size);
}

template<size_t IDX> class LayoutSize final : public Numeric {
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

class LayoutPosition : public Vec3 {
public:
    LayoutPosition(sp<View::Stub> stub, sp<Updatable> updatable, bool isBackground, bool isCenter)
        : _stub(std::move(stub)), _updatable(std::move(updatable)), _is_background(isBackground), _is_center(isCenter)
    {
    }

    bool update(uint64_t timestamp) override
    {
        return _updatable->update(timestamp);
    }

    V3 val() override
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

private:
    sp<View::Stub> _stub;
    sp<Updatable> _updatable;
    bool _is_background;
    bool _is_center;
};

class RenderableView final : public Renderable {
public:
    RenderableView(sp<View::Stub> viewStub, sp<Renderable> renderable, bool isBackground)
        : _view_stub(std::move(viewStub)), _renderable(std::move(renderable)), _is_background(isBackground)
    {
    }

    StateBits updateState(const RenderRequest& renderRequest) override
    {
        Renderable::State state = _renderable->updateState(renderRequest);
        if(state.has(Renderable::RENDERABLE_STATE_VISIBLE))
            state.set(Renderable::RENDERABLE_STATE_VISIBLE, _view_stub->isVisible());
        return state.stateBits();
    }

    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override
    {
        if(const sp<Layout::Node> topViewLayoutNode = _view_stub->getTopViewLayoutNode())
        {
            const Layout::Node& layoutNode = _view_stub->_layout_node;
            const V4& paddings = layoutNode.paddings();
            const V2& layoutSize = layoutNode.size();
            const V2 size = _is_background ? layoutSize : layoutSize - V2(paddings.y() + paddings.w(), paddings.x() + paddings.z());
            const V3 topViewOffset = _view_stub->getTopViewOffsetPosition(false);
            const V3 offset = _is_background ? topViewOffset : topViewOffset + V3(paddings.w(), paddings.x(), 0);
            Snapshot snapshot = _renderable->snapshot(snapshotContext, renderRequest, state);
            snapshot._position += V3(toViewportPosition(toPivotPosition(snapshot._model->occupy(), size) + V2(offset.x(), offset.y())), offset.z());
            snapshot._size = V3(size, 0);
            return snapshot;
        }
        return {RENDERABLE_STATE_NONE};
    }

    private:
        sp<View::Stub> _view_stub;
        sp<Renderable> _renderable;
        bool _is_background;
};

class IsDiscarded final : public Boolean {
public:
    IsDiscarded(sp<View::Stub> stub)
        : _stub(std::move(stub))
    {
    }

    bool update(uint64_t timestamp) override
    {
        bool dirty = false;
        sp<View::Stub> stub = _stub;
        while(stub)
        {
            dirty = stub->_discarded.update(timestamp) || dirty;
            stub = stub->_parent_stub.lock();
        }
        return dirty;
    }
    bool val() override
    {
        return _stub->isDiscarded();
    }

private:
    sp<View::Stub> _stub;
};

class UpdatableIsolatedLayout : public Updatable {
public:
    UpdatableIsolatedLayout(sp<View::Stub> stub)
        : _stub(std::move(stub))
    {
    }

    bool update(uint64_t timestamp) override
    {
        sp<View::Stub> stub = _stub;
        while(stub)
        {
            if(stub->_top_view)
                break;
            if(stub->_hierarchy && stub->_hierarchy->isIsolatedLayout())
                break;

            stub = stub->_parent_stub.lock();
        }
        return stub ? stub->update(timestamp) : false;
    }

private:
    sp<View::Stub> _stub;
};

}

View::View(sp<LayoutParam> layoutParam, sp<RenderObject> background, sp<Boolean> visible, sp<Boolean> discarded)
    : _stub(sp<Stub>::make(std::move(layoutParam), std::move(visible), std::move(discarded))), _background(std::move(background)), _is_discarded(sp<Boolean>::make<IsDiscarded>(_stub)),
      _updatable_view(sp<UpdatableOncePerFrame>::make(_stub)), _updatable_layout(sp<UpdatableOncePerFrame>::make(sp<UpdatableIsolatedLayout>::make(_stub)))
{
}

View::~View()
{
    _stub->dispose();
}

TypeId View::onPoll(WiringContext& context)
{
    sp<Vec3> size = Vec3Type::create(sp<LayoutSize<0>>::make(_stub), sp<LayoutSize<1>>::make(_stub), Global<Constants>()->NUMERIC_ZERO);
    context.addComponentBuilder(make_lazy_builder<Shape>(Shape::TYPE_AABB, sp<Vec3>(size)));
    context.addComponentBuilder(make_lazy_builder<Boundaries>(std::move(size)));
    context.addComponentBuilder(make_lazy_builder<Vec3, LayoutPosition>(_stub, _updatable_layout, true, true));
    if(_background)
        context.addComponentBuilder(to_lazy_builder<Renderable>(RenderableType::create, sp<Renderable>::make<RenderableView>(_stub, _background, true), _updatable_layout, _is_discarded));
    return Type<View>::id();
}

void View::onWire(const WiringContext& context)
{
}

bool View::updateLayout(uint64_t timestamp) const
{
    return _updatable_view->update(timestamp);
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
    return _stub->_layout_node->_layout_param;
}

void View::setLayoutParam(sp<LayoutParam> layoutParam)
{
    _stub->_layout_node->_layout_param = std::move(layoutParam);
}

void View::addView(sp<View> view, sp<Boolean> discarded)
{
    if(discarded)
        view->setDiscarded(std::move(discarded));

    view->setParent(*this);
    _stub->ensureViewHierarchy().addView(std::move(view));
}

const sp<ViewHierarchy>& View::hierarchy() const
{
    return _stub->_hierarchy;
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
    : _factory(factory), _manifest(manifest), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED)), _visible(factory.getBuilder<Boolean>(manifest, constants::VISIBLE)),
      _background(factory.getBuilder<RenderObject>(manifest, constants::BACKGROUND)), _layout_param(factory.ensureConcreteClassBuilder<LayoutParam>(manifest, "layout-param"))
{
}

sp<View> View::BUILDER::build(const Scope& args)
{
    sp<View> view = sp<View>::make(_layout_param.build(args), _background.build(args), _visible.build(args), _discarded.build(args));
    for(const document& i : _manifest->children(constants::VIEW))
        view->addView(_factory.ensure<View>(i, args));
    return view;
}

View::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest)
{
}

sp<Wirable> View::BUILDER_WIRABLE::build(const Scope& args)
{
    return _builder_impl.build(args);
}

View::BUILDER_VIEW::BUILDER_VIEW(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest), _builder_text(factory, manifest)
{
}

sp<View> View::BUILDER_VIEW::build(const Scope& args)
{
    const sp<View> view = _builder_impl.build(args);
    const sp<Text> text = _builder_text.build(args);
    Traits traits(view, sp<Wirable>::make<WithText>(text));
    WirableType::wireAll(traits);
    text->show(sp<Boolean>::make<BooleanByWeakRef<View>>(view, 0));
    return view;
}

}
