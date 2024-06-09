#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/util/numeric_type.h"
#include "core/util/updatable_util.h"

#include "graphics/traits/with_layer.h"

#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"
#include "renderer/base/varyings.h"

namespace ark {

RenderObject::RenderObject(int32_t type, sp<Vec3> position, sp<Size> size, sp<Transform> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> discarded)
    : RenderObject(sp<IntegerWrapper>::make(type), std::move(position), std::move(size), std::move(transform), std::move(varyings), std::move(visible), std::move(discarded))
{
}

RenderObject::RenderObject(sp<Integer> type, sp<Vec3> position, sp<Size> size, sp<Transform> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> discarded)
    : _type(sp<IntegerWrapper>::make(std::move(type))), _position(std::move(position)), _size(std::move(size)), _transform(std::move(transform)), _varyings(std::move(varyings)), _visible(std::move(visible), true), _discarded(std::move(discarded), false)
{
}

sp<Integer> RenderObject::type() const
{
    return _type;
}

float RenderObject::width()
{
    return _size.val().x();
}

float RenderObject::height()
{
    return _size.val().y();
}

const SafeVar<Vec3>& RenderObject::size()
{
    return _size;
}

const SafePtr<Transform>& RenderObject::transform() const
{
    return _transform;
}

const sp<Varyings>& RenderObject::varyings()
{
    if(!_varyings)
        setVaryings(sp<Varyings>::make());
    return _varyings;
}

void RenderObject::setType(int32_t type)
{
    _type->set(type);
    _discarded.reset(nullptr);
    _timestamp.markDirty();
}

void RenderObject::setType(const sp<Integer>& type)
{
    _type->set(type);
    _timestamp.markDirty();
}

float RenderObject::x() const
{
    return _position.val().x();
}

void RenderObject::setX(float x)
{
    Vec3Type::setX(_position.ensure(), x);
    _timestamp.markDirty();
}

void RenderObject::setX(const sp<Numeric>& x)
{
    Vec3Type::setX(_position.ensure(), x);
    _timestamp.markDirty();
}

float RenderObject::y() const
{
    return _position.val().y();
}

void RenderObject::setY(float y)
{
    Vec3Type::setY(_position.ensure(), y);
    _timestamp.markDirty();
}

void RenderObject::setY(const sp<Numeric>& y)
{
    Vec3Type::setY(_position.ensure(), y);
    _timestamp.markDirty();
}

float RenderObject::z() const
{
    return _position.val().z();
}

void RenderObject::setZ(float z)
{
    Vec3Type::setZ(_position.ensure(), z);
    _timestamp.markDirty();
}

void RenderObject::setZ(const sp<Numeric>& z)
{
    Vec3Type::setZ(_position.ensure(), z);
    _timestamp.markDirty();
}

V2 RenderObject::xy() const
{
    const V3 xyz = _position.val();
    return V2(xyz.x(), xyz.y());
}

V3 RenderObject::xyz() const
{
    return _position.val();
}

const sp<Vec3>& RenderObject::position()
{
    return _position.ensure();
}

void RenderObject::setPosition(const sp<Vec3>& position)
{
    _position = position;
    _timestamp.markDirty();
}

void RenderObject::setSize(sp<Vec3> size)
{
    _size.reset(std::move(size));
    _timestamp.markDirty();
}

void RenderObject::setTransform(const sp<Transform>& transform)
{
    _transform = transform;
    _timestamp.markDirty();
}

void RenderObject::setVaryings(sp<Varyings> varyings)
{
    _varyings = varyings;
    _timestamp.markDirty();
}

const Box& RenderObject::tag() const
{
    return _tag;
}

void RenderObject::setTag(const Box& tag)
{
    _tag = tag;
}

const sp<Boolean>& RenderObject::discarded()
{
    return _discarded.ensure();
}

void RenderObject::setDiscarded(sp<Boolean> discarded)
{
    _discarded.reset(std::move(discarded));
    _timestamp.markDirty();
}

sp<Boolean> RenderObject::visible()
{
    return _visible.ensure();
}

void RenderObject::setVisible(bool visible)
{
    _visible.reset(sp<Boolean::Const>::make(visible));
    _timestamp.markDirty();
}

void RenderObject::setVisible(sp<Boolean> visible)
{
    _visible.reset(std::move(visible));
    _timestamp.markDirty();
}

void RenderObject::dispose()
{
    setDiscarded(sp<Boolean::Const>::make(true));
}

void RenderObject::show()
{
    setVisible(true);
}

void RenderObject::hide()
{
    setVisible(false);
}

bool RenderObject::isDiscarded() const
{
    return _type->val() < 0 || _discarded.val();
}

bool RenderObject::isVisible() const
{
    return _visible.val();
}

Renderable::StateBits RenderObject::updateState(const RenderRequest& renderRequest)
{
    bool dirty = _timestamp.update(renderRequest.timestamp());
    if((_discarded.update(renderRequest.timestamp()) || dirty) && _discarded.val())
        return Renderable::RENDERABLE_STATE_DISCARDED;

    dirty = UpdatableUtil::update(renderRequest.timestamp(), _visible, _type, _position, _size, _transform, _varyings, _visible) || dirty;
    return static_cast<Renderable::StateBits>((_type->val() == -1 ? Renderable::RENDERABLE_STATE_DISCARDED : 0) | (dirty ? Renderable::RENDERABLE_STATE_DIRTY : 0) |
                                              (_visible.val() ? Renderable::RENDERABLE_STATE_VISIBLE : 0));
}

Renderable::Snapshot RenderObject::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    const int32_t typeId = _type->val();
    sp<Model> model = snapshotContext._render_layer.context()->modelLoader()->loadModel(typeId);
    if(state & RENDERABLE_STATE_DIRTY)
        return {state, typeId, std::move(model), _position.val(), _size.val(), _transform->snapshot(V3(0)), _varyings ? _varyings->snapshot(snapshotContext.pipelineInput(), renderRequest.allocator()) : Varyings::Snapshot()};
    return {state, typeId, std::move(model)};
}

TypeId RenderObject::onPoll(WiringContext& /*context*/)
{
    return Type<Renderable>::id();
}

void RenderObject::onWire(const WiringContext& context)
{
    if(sp<Vec3> position = context.getComponent<Vec3>())
        _position.reset(std::move(position));
    if(!_size)
        if(const sp<Size> size = context.getComponent<Size>())
            _size.reset(size);
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(factory.getBuilder<Integer>(manifest, constants::TYPE)),
      _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)),
      _size(factory.getBuilder<Size>(manifest, constants::SIZE)),
      _transform(factory.getBuilder<Transform>(manifest, constants::TRANSFORM)),
      _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, constants::VARYINGS)),
      _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED))
{
}

sp<RenderObject> RenderObject::BUILDER::build(const Scope& args)
{
    return sp<RenderObject>::make(_type->build(args), _position->build(args), _size->build(args), _transform->build(args), _varyings->build(args), nullptr, _discarded->build(args));
}

RenderObject::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest)
{
}

sp<Wirable> RenderObject::BUILDER_WIRABLE::build(const Scope& args)
{
    return _builder_impl.build(args);
}

}
