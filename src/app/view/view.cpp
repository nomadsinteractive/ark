#include "app/view/view.h"

#include "core/base/bean_factory.h"
#include "core/components/discarded.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/types/global.h"
#include "core/util/math.h"
#include "core/util/updatable_util.h"

#include "graphics/components/layer.h"
#include "graphics/components/layout_param.h"
#include "graphics/components/translation.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "graphics/components/shape.h"
#include "app/view/view_hierarchy.h"

namespace ark {

struct View::Stub {
    sp<Node> _node;

    sp<Updatable> _updatable_layout;
    sp<Vec3> _layout_position;
    sp<Size> _layout_size;
};

struct View::Node final : Updatable {
    Node(sp<LayoutParam> layoutParam, String name, sp<Boolean> discarded)
        : _name(std::move(name)), _layout_node(sp<Layout::Node>::make(std::move(layoutParam))), _hierarchy(sp<ViewHierarchy>::make(_layout_node->_layout_param->layout(), _layout_node)),
          _discarded(std::move(discarded), false), _top_view(false)
    {
    }

    bool update(const uint32_t tick) override
    {
        if(_hierarchy->layout())
            return _hierarchy->updateLayout(tick);

        return UpdatableUtil::update(tick, _layout_node->_layout_param, _discarded);
    }

    V3 getTopViewOffsetPosition() const
    {
        const Layout::Node& layoutNode = _layout_node;
        const V3 layoutOffset(layoutNode.offsetPosition(), 0);
        const sp<Stub> parentStub = _parent_stub.lock();
        const V3 offset = (parentStub ? parentStub->_node->getTopViewOffsetPosition() + layoutOffset : layoutOffset);
        return layoutNode._layout_param->offset().val() + offset;
    }

    sp<Layout::Node> getTopViewLayoutNode() const
    {
        if(_top_view)
            return _layout_node;
        const sp<Stub> parentStub = _parent_stub.lock();
        return parentStub ? parentStub->_node->getTopViewLayoutNode() : nullptr;
    }

    String _name;
    sp<Layout::Node> _layout_node;
    sp<ViewHierarchy> _hierarchy;
    OptionalVar<Boolean> _discarded;

    WeakPtr<View::Stub> _parent_stub;
    bool _top_view;
};

namespace {

V2 toViewportPosition(const V2& position)
{
    return Ark::instance().applicationContext()->renderEngine()->toLHSPosition(position);
}

sp<View::Stub> findLayoutTopView(sp<View::Stub> stub)
{
    while(stub)
    {
        if(stub->_node->_top_view)
            break;
        if(stub->_node->_hierarchy && stub->_node->_hierarchy->isLayoutTopView())
            break;

        stub = stub->_node->_parent_stub.lock();
    }
    return stub;
}

class UpdatableOncePerTick final : public Updatable {
public:
    UpdatableOncePerTick(sp<Updatable> delegate)
        : _delegate(std::move(delegate)), _last_tick(Timestamp::now() - 1), _last_value(false) {
    }

    bool update(const uint32_t tick) override
    {
        if(_last_tick != tick)
        {
            _last_tick = tick;
            _last_value = _delegate->update(tick);
        }
        return _last_value;
    }

private:
    sp<Updatable> _delegate;
    uint32_t _last_tick;
    bool _last_value;
};

template<size_t IDX> class LayoutSize final : public Numeric {
public:
    LayoutSize(sp<View::Node> stub, sp<Updatable> updatable)
        : _stub(std::move(stub)), _updatable(std::move(updatable)) {
    }

    bool update(const uint32_t tick) override {
        const bool dirty = _updatable->update(tick);
        return _stub->_layout_node ? _stub->_layout_node->size().update(tick) | dirty : dirty;
    }

    float val() override {
        if(!_stub->_layout_node)
            return 0;

        const V2& size = _stub->_layout_node->size();
        return size[IDX];
    }

private:
    sp<View::Node> _stub;
    sp<Updatable> _updatable;
};

class LayoutPosition final : public Vec3 {
public:
    LayoutPosition(sp<View::Node> stub, sp<Updatable> updatable)
        : _hierarchy(std::move(stub)), _updatable(std::move(updatable))
    {
    }

    bool update(const uint32_t tick) override
    {
        return _updatable->update(tick);
    }

    V3 val() override
    {
        if(!_hierarchy->_layout_node)
            return V3();

        const Layout::Node& layoutNode = _hierarchy->_layout_node;
        const V2& size = layoutNode.size();
        const V3 offsetPosition = _hierarchy->getTopViewOffsetPosition();
        const float x = offsetPosition.x() + size.x() / 2;
        const float y = offsetPosition.y() + size.y() / 2;
        return {toViewportPosition({x, y}), offsetPosition.z()};
    }

private:
    sp<View::Node> _hierarchy;
    sp<Updatable> _updatable;
};

class UpdatableLayoutTopView final : public Updatable {
public:
    UpdatableLayoutTopView(sp<View::Stub> stub)
        : _stub(std::move(stub))
    {
    }

    bool update(const uint32_t tick) override
    {
        const sp<View::Stub> stub = findLayoutTopView(_stub);
        return stub ? stub->_node->update(tick) : false;
    }

private:
    sp<View::Stub> _stub;
};

}

View::View(sp<LayoutParam> layoutParam, String name, sp<Boolean> discarded)
    : _stub(sp<Stub>::make(Stub{sp<Node>::make(std::move(layoutParam), std::move(name), std::move(discarded))}))
{
}

View::View(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

void View::onPoll(WiringContext& context)
{
    context.setComponent(layoutSize());
    context.setComponent(sp<Shape>::make(Shape::TYPE_AABB, layoutSize()->val()));
}

void View::onWire(const WiringContext& context, const Box& self)
{
    if(sp<Boolean> discarded = context.getComponent<Discarded>())
        setDiscarded(std::move(discarded));
}

void View::onPoll(WiringContext& context, const document& component)
{
    if(const String& viewName = Documents::getAttribute(component, "view-name"))
    {
        const sp<View> view = findView(viewName);
        view->onPoll(context);
        context.setComponent(view);
    }
}

bool View::update(const uint32_t tick) const
{
    return _stub->_node->update(tick);
}

const sp<Layout::Node>& View::layoutNode() const
{
    return _stub->_node->_layout_node;
}

const String& View::name() const
{
    return _stub->_node->_name;
}

const OptionalVar<Boolean>& View::discarded() const
{
    return _stub->_node->_discarded;
}

void View::setDiscarded(sp<Boolean> discarded) const
{
    _stub->_node->_discarded.reset(std::move(discarded));
}

const sp<LayoutParam>& View::layoutParam() const
{
    return _stub->_node->_layout_node->_layout_param;
}

void View::setLayoutParam(sp<LayoutParam> layoutParam) const
{
    _stub->_node->_layout_node->_layout_param = std::move(layoutParam);
}

const sp<Vec3>& View::layoutPosition()
{
    if(!_stub->_layout_position)
        _stub->_layout_position = sp<Vec3>::make<LayoutPosition>(_stub->_node, ensureUpdatableLayout());
    return _stub->_layout_position;
}

const sp<Size>& View::layoutSize()
{
    if(!_stub->_layout_size)
        _stub->_layout_size = sp<Size>::make(sp<LayoutSize<0>>::make(_stub->_node, ensureUpdatableLayout()), sp<LayoutSize<1>>::make(_stub->_node, ensureUpdatableLayout()), Global<Constants>()->NUMERIC_ZERO);
    return _stub->_layout_size;
}

void View::addView(sp<View> view) const
{
    view->setParent(*this);
    _stub->_node->_hierarchy->addView(std::move(view));

    if(const sp<Stub> layoutTopView = findLayoutTopView(_stub))
        layoutTopView->_node->_hierarchy->markHierarchyDirty();
}

sp<View> View::findView(const StringView name) const
{
    if(this->name() == name)
        return sp<View>::make(_stub);

    if(_stub->_node->_hierarchy)
        for(const sp<View>& i : _stub->_node->_hierarchy->updateChildren())
        {
            if(i->_stub->_node->_name == name)
                return i;
            if(sp<View> found = i->findView(name))
                return found;
        }
    return nullptr;
}

sp<Boundaries> View::makeBoundaries()
{
    const sp<Vec3>& position = layoutPosition();
    const sp<Vec3> extent = Vec3Type::mul(layoutSize(), 0.5f);
    return sp<Boundaries>::make(Vec3Type::sub(position, extent), Vec3Type::add(position, extent), position, layoutSize());
}

const sp<ViewHierarchy>& View::hierarchy() const
{
    return _stub->_node->_hierarchy;
}

sp<View> View::parent() const
{
    if(sp<Stub> stub = _stub->_node->_parent_stub.lock())
        return sp<View>::make(std::move(stub));
    return nullptr;
}

void View::setParent(const View& view)
{
    _stub->_node->_parent_stub = view._stub;
}

Vector<sp<View>> View::children() const
{
    if(_stub->_node->_hierarchy)
        return _stub->_node->_hierarchy->updateChildren();
    return {};
}

sp<Updatable>& View::ensureUpdatableLayout()
{
    if(!_stub->_updatable_layout)
        _stub->_updatable_layout = sp<Updatable>::make<UpdatableOncePerTick>(sp<Updatable>::make<UpdatableLayoutTopView>(_stub));
    return _stub->_updatable_layout;
}

void View::markAsTopView()
{
    _stub->_node->_top_view = true;
    _stub->_node->_parent_stub.reset(nullptr);
}

View::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _name(Documents::getAttribute(manifest, constants::NAME)), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED)),
      _layout_param(factory.ensureConcreteClassBuilder<LayoutParam>(manifest, constants::LAYOUT_PARAM)), _children(factory.makeBuilderList<View>(manifest, constants::VIEW))
{
    CHECK(!factory.getBuilder<Vec3>(manifest, constants::POSITION), "Setting position of view is no longer supported, use offset as an alternative.");
}

sp<View> View::BUILDER::build(const Scope& args)
{
    sp<View> view = sp<View>::make(_layout_param.build(args), _name, _discarded.build(args));
    for(const builder<View>& i : _children)
        view->addView(i->build(args));
    return view;
}

View::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _view(factory.ensureBuilder<View>(manifest))
{
}

sp<Wirable> View::BUILDER_WIRABLE::build(const Scope& args)
{
    return _view->build(args);
}

}
