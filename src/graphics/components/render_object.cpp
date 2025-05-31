#include "graphics/components/render_object.h"

#include "core/base/bean_factory.h"
#include "core/base/named_hash.h"
#include "core/components/discarded.h"
#include "core/components/with_id.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/inf/wirable.h"
#include "core/util/updatable_util.h"

#include "graphics/impl/transform/transform_impl.h"
#include "graphics/base/transform_3d.h"
#include "graphics/components/rotation.h"
#include "graphics/components/translation.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"
#include "renderer/base/shader.h"
#include "renderer/components/varyings.h"

#include "app/view/view.h"
#include "core/components/tags.h"

namespace ark {

RenderObject::RenderObject(const NamedHash& type, sp<Vec3> position, sp<Vec3> size, sp<Transform> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> discarded)
    : RenderObject(sp<IntegerWrapper>::make(type.hash()), std::move(position), std::move(size), std::move(transform), std::move(varyings), std::move(visible), std::move(discarded))
{
}

RenderObject::RenderObject(sp<Integer> type, sp<Vec3> position, sp<Vec3> size, sp<Transform> transform, sp<Varyings> varyings, sp<Boolean> visible, sp<Boolean> discarded)
    : _type(sp<IntegerWrapper>::make(std::move(type))), _position(std::move(position)), _size(std::move(size)), _transform(transform ? std::move(transform) : sp<Transform>::make<TransformImpl>(TransformType::TYPE_LINEAR_3D)),
      _varyings(std::move(varyings)), _visible(std::move(visible), true), _discarded(std::move(discarded), false)
{
}

sp<Integer> RenderObject::type() const
{
    return _type;
}

const SafeVar<Vec3>& RenderObject::size()
{
    return _size;
}

const sp<Transform>& RenderObject::transform() const
{
    return _transform;
}

const sp<Varyings>& RenderObject::varyings()
{
    if(!_varyings)
        setVaryings(sp<Varyings>::make());
    return _varyings;
}

void RenderObject::setType(const NamedHash& type)
{
    _type->set(type.hash());
    _timestamp.markDirty();
}

void RenderObject::setType(sp<Integer> type)
{
    _type->set(std::move(type));
    _timestamp.markDirty();
}

sp<Vec3> RenderObject::position()
{
    return _position.ensure();
}

void RenderObject::setPosition(sp<Vec3> position)
{
    _position.reset(std::move(position));
    _timestamp.markDirty();
}

void RenderObject::setSize(sp<Vec3> size)
{
    _size.reset(std::move(size));
    _timestamp.markDirty();
}

void RenderObject::setTransform(sp<Transform> transform)
{
    _transform = std::move(transform);
    _timestamp.markDirty();
}

void RenderObject::setVaryings(sp<Varyings> varyings)
{
    _varyings = std::move(varyings);
    _timestamp.markDirty();
}

Box RenderObject::tag() const
{
    return _tags ? _tags->tag() : Box();
}

void RenderObject::setTag(Box tag)
{
    if(_tags)
        _tags->setTag(0, std::move(tag));
    else
        _tags = sp<Tags>::make(std::move(tag));
}

sp<Boolean> RenderObject::discarded()
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

void RenderObject::discard()
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
        return RENDERABLE_STATE_DISCARDED;

    dirty = UpdatableUtil::update(renderRequest.timestamp(), _visible, _type, _position, _size, _transform, _varyings, _visible) || dirty;
    return static_cast<StateBits>((dirty ? RENDERABLE_STATE_DIRTY : 0) | (_visible.val() ? RENDERABLE_STATE_VISIBLE : 0));
}

Renderable::Snapshot RenderObject::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    const int32_t typeId = _type->val();
    sp<Model> model = snapshotContext._render_layer.modelLoader()->loadModel(typeId);
    if(state & RENDERABLE_STATE_DIRTY)
        return {state, typeId, std::move(model), _position.val(), _size.val(), _transform, _transform->snapshot(), _varyings ? _varyings->snapshot(snapshotContext.pipelineInput(), renderRequest.allocator()) : Varyings::Snapshot()};
    return {state, typeId, std::move(model)};
}

void RenderObject::onWire(const WiringContext& context, const Box& self)
{
    const sp<View> view = context.getComponent<View>();

    if(sp<Vec3> size = context.getComponent<Size>(); size && !view)
        setSize(std::move(size));

    if(view)
    {
        setPosition(view->layoutPosition());
        setSize(view->layoutSize());
    }
    else if(const auto boundaries = context.getComponent<Boundaries>())
        setPosition(boundaries->center());
    else if(sp<Vec3> translation = context.getComponent<Translation>())
        setPosition(std::move(translation));

    if(auto transform = context.getComponent<Transform>())
        setTransform(std::move(transform));
    else
    {
        sp<Vec4> rotation = context.getComponent<Rotation>();
        //TODO: Scale and translation
        if(rotation)
            setTransform(sp<Transform>::make<Transform3D>(std::move(rotation)));
    }

    if(const sp<Visibility> visibility = context.getComponent<Visibility>(); visibility && !_visible)
        setVisible(visibility);

    if(sp<Varyings> varyings = context.getComponent<Varyings>())
        setVaryings(std::move(varyings));

    if(sp<Boolean> discarded = context.getComponent<Discarded>(); discarded && !_discarded)
        setDiscarded(std::move(discarded));

    if(auto tags = context.getComponent<Tags>())
        _tags = std::move(tags);

    if(const auto layer = context.getComponent<Layer>())
    {
        if(const enums::LayerPushOrder lpo = context.getEnum<enums::LayerPushOrder>(enums::LayerPushOrder::LAYER_PUSH_ORDER_BACK);
            lpo == enums::LAYER_PUSH_ORDER_BACK)
            layer->pushBack(self.as<RenderObject>());
        else
            layer->pushFront(self.as<RenderObject>());

        if(const sp<WithId>& withId = context.getComponent<WithId>(); withId && layer->shader() && layer->shader()->layout()->getAttribute("Id"))
            varyings()->setProperty(constants::ID, sp<Integer>::make<Integer::Const>(withId->id()));
    }
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(factory.getBuilder<Integer>(manifest, constants::TYPE)), _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)), _size(factory.getBuilder<Size>(manifest, constants::SIZE)),
      _transform(factory.getBuilder<Transform>(manifest, constants::TRANSFORM)), _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, constants::VARYINGS)),
      _discarded(factory.getBuilder<Boolean>(manifest, constants::DISCARDED))
{
}

sp<RenderObject> RenderObject::BUILDER::build(const Scope& args)
{
    return sp<RenderObject>::make(_type.build(args), _position.build(args), _size.build(args), _transform.build(args), _varyings.build(args), nullptr, _discarded.build(args));
}

RenderObject::BUILDER_RENDERABLE::BUILDER_RENDERABLE(BeanFactory& factory, const document& manifest)
    : _builder_impl(factory, manifest)
{
}

sp<Renderable> RenderObject::BUILDER_RENDERABLE::build(const Scope& args)
{
    return _builder_impl.build(args);
}

RenderObject::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _render_object(factory.ensureBuilder<RenderObject>(manifest))
{
}

sp<Wirable> RenderObject::BUILDER_WIRABLE::build(const Scope& args)
{
    return _render_object->build(args);
}

}
