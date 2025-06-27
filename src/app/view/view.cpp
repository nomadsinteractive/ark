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
#include "app/components/shape.h"
#include "app/view/view_hierarchy.h"

namespace ark {

struct View::Stub final : Updatable {
    Stub(sp<LayoutParam> layoutParam, String name, sp<Vec3> position, sp<Boolean> discarded)
        : _name(std::move(name)), _hierarchy(layoutParam->layout() ? sp<ViewHierarchy>::make(layoutParam->layout()) : nullptr), _layout_node(sp<Layout::Node>::make(std::move(layoutParam))), _position(std::move(position)),
          _discarded(std::move(discarded), false), _top_view(false)
    {
    }

    bool update(const uint64_t timestamp) override
    {
        if(_hierarchy)
        {
            const bool positionDirty = _position.update(timestamp);
            return _hierarchy->updateLayout(_layout_node, timestamp) || positionDirty;
        }
        return UpdatableUtil::update(timestamp, _layout_node->_layout_param, _position, _discarded);
    }

    void discard()
    {
        _discarded.reset(sp<Boolean>::make<Boolean::Const>(true));
    }

    bool isDiscarded() const
    {
        const sp<Stub> parentStub = _parent_stub.lock();
        return _discarded.val() || (parentStub ? parentStub->isDiscarded() : !_top_view);
    }

    V3 getTopViewOffsetPosition(const bool includePaddings) const
    {
        const Layout::Node& layoutNode = _layout_node;
        const V3 layoutOffset(layoutNode.offsetPosition(), 0);
        const sp<Stub> parentStub = _parent_stub.lock();
        V3 offset = (parentStub ? parentStub->getTopViewOffsetPosition(false) : getOffsetPosition()) + layoutOffset;
        if(includePaddings)
            offset += V3(layoutNode.paddings().w(), layoutNode.paddings().x(), 0);
        return layoutNode._layout_param->offset().val() + offset;
    }

    V3 getOffsetPosition() const
    {
        if(!_position)
            return V3();

        const V3 position = _position.val();
        const V2 layoutSize = _layout_node->size().val();
        return {position.x() - layoutSize.x() / 2, position.y() - layoutSize.y() / 2, position.z()};
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

    String _name;
    sp<ViewHierarchy> _hierarchy;
    sp<Layout::Node> _layout_node;

    SafeVar<Vec3> _position;
    SafeVar<Boolean> _discarded;

    WeakPtr<Stub> _parent_stub;
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
        if(stub->_top_view)
            break;
        if(stub->_hierarchy && stub->_hierarchy->isLayoutTopView())
            break;

        stub = stub->_parent_stub.lock();
    }
    return stub;
}

template<size_t IDX> class LayoutSize final : public Numeric {
public:
    LayoutSize(sp<View::Stub> stub, sp<Updatable> updatable)
        : _stub(std::move(stub)), _updatable(std::move(updatable)) {
    }

    bool update(const uint64_t timestamp) override {
        const bool dirty = _updatable->update(timestamp);
        return _stub->_layout_node ? _stub->_layout_node->size().update(timestamp) | dirty : dirty;
    }

    float val() override {
        if(!_stub->_layout_node)
            return 0;

        const V2& size = _stub->_layout_node->size();
        return size[IDX];
    }

private:
    sp<View::Stub> _stub;
    sp<Updatable> _updatable;
};

class LayoutPosition final : public Vec3 {
public:
    LayoutPosition(sp<View::Stub> stub, sp<Updatable> updatable)
        : _stub(std::move(stub)), _updatable(std::move(updatable))
    {
    }

    bool update(const uint64_t timestamp) override
    {
        return _updatable->update(timestamp);
    }

    V3 val() override
    {
        if(!_stub->_layout_node)
            return V3();

        const Layout::Node& layoutNode = _stub->_layout_node;
        const V2& size = layoutNode.size();
        const V3 offsetPosition = _stub->getTopViewOffsetPosition(false);
        const float x = offsetPosition.x() + size.x() / 2;
        const float y = offsetPosition.y() + size.y() / 2;
        return {toViewportPosition({x, y}), offsetPosition.z()};
    }

private:
    sp<View::Stub> _stub;
    sp<Updatable> _updatable;
};

class UpdatableLayoutTopView final : public Updatable {
public:
    UpdatableLayoutTopView(sp<View::Stub> stub)
        : _stub(std::move(stub))
    {
    }

    bool update(const uint64_t timestamp) override
    {
        const sp<View::Stub> stub = findLayoutTopView(_stub);
        return stub ? stub->update(timestamp) : false;
    }

private:
    sp<View::Stub> _stub;
};

}

View::View(sp<LayoutParam> layoutParam, String name, sp<Vec3> position, sp<Boolean> discarded)
    : _stub(sp<Stub>::make(std::move(layoutParam), std::move(name), std::move(position), std::move(discarded)))
{
}

View::~View()
{
    _stub->discard();
}

TypeId View::onPoll(WiringContext& context)
{
    context.setComponent(layoutSize());
    context.setComponent(sp<Shape>::make(Shape::TYPE_AABB, layoutSize()));
    return constants::TYPE_ID_NONE;
}

void View::onWire(const WiringContext& context, const Box& self)
{
    if(sp<Vec3> translation = context.getComponent<Translation>())
        _stub->_position.reset(std::move(translation));

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

bool View::update(const uint64_t timestamp) const
{
    return _stub->update(timestamp);
}

const sp<Layout::Node>& View::layoutNode() const
{
    return _stub->_layout_node;
}

const String& View::name() const
{
    return _stub->_name;
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

const sp<Vec3>& View::layoutPosition()
{
    if(!_layout_position)
        _layout_position = sp<LayoutPosition>::make(_stub, ensureUpdatableLayout());
    return _layout_position;
}

const sp<Size>& View::layoutSize()
{
    if(!_layout_size)
        _layout_size = sp<Size>::make(sp<LayoutSize<0>>::make(_stub, ensureUpdatableLayout()), sp<LayoutSize<1>>::make(_stub, ensureUpdatableLayout()), Global<Constants>()->NUMERIC_ZERO);
    return _layout_size;
}

void View::addView(sp<View> view)
{
    view->setParent(*this);
    _stub->ensureViewHierarchy().addView(std::move(view));

    if(const sp<Stub> layoutTopView = findLayoutTopView(_stub))
        if(layoutTopView->_hierarchy)
            layoutTopView->_hierarchy->markHierarchyDirty();
}

sp<View> View::findView(const StringView name) const
{
    if(_stub->_hierarchy)
        for(const sp<View>& i : _stub->_hierarchy->updateChildren())
        {
            if(i->_stub->_name == name)
                return i;
            if(sp<View> found = i->findView(name))
                return found;
        }
    return nullptr;
}

sp<Boundaries> View::makeBoundaries()
{
    return sp<Boundaries>::make(layoutPosition(), Vec3Type::mul(layoutSize(), 0.5f));
}

const sp<ViewHierarchy>& View::hierarchy() const
{
    return _stub->_hierarchy;
}

void View::setParent(const View& view)
{
    _stub->_parent_stub = view._stub;
}

sp<Updatable>& View::ensureUpdatableLayout()
{
    if(!_updatable_layout)
        _updatable_layout = sp<Updatable>::make<UpdatableLayoutTopView>(_stub);
    return _updatable_layout;
}

void View::markAsTopView()
{
    _stub->_top_view = true;
    _stub->_parent_stub.reset(nullptr);
}

View::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _name(Documents::getAttribute(manifest, constants::NAME)), _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)), _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED)),
      _layout_param(factory.ensureConcreteClassBuilder<LayoutParam>(manifest, constants::LAYOUT_PARAM)), _children(factory.makeBuilderList<View>(manifest, constants::VIEW))
{
}

sp<View> View::BUILDER::build(const Scope& args)
{
    sp<View> view = sp<View>::make(_layout_param.build(args), _name, _position.build(args), _discarded.build(args));
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
