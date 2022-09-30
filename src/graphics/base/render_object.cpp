#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
#include "core/util/numeric_type.h"
#include "core/util/variable_util.h"

#include "graphics/util/vec3_type.h"

#include "renderer/base/varyings.h"

namespace ark {

RenderObject::RenderObject(int32_t type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings, sp<Visibility> visible, sp<Disposed> disposed)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _visible(std::move(visible), true), _disposed(std::move(disposed), false)
{
}

RenderObject::RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings, sp<Visibility> visible, sp<Disposed> disposed)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _visible(std::move(visible), true), _disposed(std::move(disposed), false)
{
}

sp<Integer> RenderObject::type() const
{
    return _type;
}

float RenderObject::width()
{
    return _size.ensure()->width();
}

float RenderObject::height()
{
    return _size.ensure()->height();
}

const sp<Size>& RenderObject::size()
{
    return _size.ensure();
}

void RenderObject::traverse(const Holder::Visitor& visitor)
{
    visitor(_tag);
    HolderUtil::visit(_type->delegate(), visitor);
    HolderUtil::visit(_position.delegate(), visitor);
    HolderUtil::visit(_size.delegate(), visitor);
    HolderUtil::visit(_transform, visitor);
    HolderUtil::visit(_varyings, visitor);
    HolderUtil::visit(_disposed.delegate(), visitor);
    HolderUtil::visit(_visible.delegate(), visitor);
}

const SafePtr<Transform>& RenderObject::transform() const
{
    return _transform;
}

const SafePtr<Varyings>& RenderObject::varyings() const
{
    return _varyings;
}

void RenderObject::setType(int32_t type)
{
    _type->set(type);
    _disposed.reset(nullptr);
    _timestamp.setDirty();
}

void RenderObject::setType(const sp<Integer>& type)
{
    _type->set(type);
    _disposed = type.as<Disposed>();
    _timestamp.setDirty();
}

float RenderObject::x() const
{
    return _position.val().x();
}

void RenderObject::setX(float x)
{
    Vec3Type::setX(_position.ensure(), x);
    _timestamp.setDirty();
}

void RenderObject::setX(const sp<Numeric>& x)
{
    Vec3Type::setX(_position.ensure(), x);
    _timestamp.setDirty();
}

float RenderObject::y() const
{
    return _position.val().y();
}

void RenderObject::setY(float y)
{
    Vec3Type::setY(_position.ensure(), y);
    _timestamp.setDirty();
}

void RenderObject::setY(const sp<Numeric>& y)
{
    Vec3Type::setY(_position.ensure(), y);
    _timestamp.setDirty();
}

float RenderObject::z() const
{
    return _position.val().z();
}

void RenderObject::setZ(float z)
{
    Vec3Type::setZ(_position.ensure(), z);
    _timestamp.setDirty();
}

void RenderObject::setZ(const sp<Numeric>& z)
{
    Vec3Type::setZ(_position.ensure(), z);
    _timestamp.setDirty();
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
    _timestamp.setDirty();
}

void RenderObject::setSize(const sp<Size>& size)
{
    _size = size;
    _timestamp.setDirty();
}

void RenderObject::setTransform(const sp<Transform>& transform)
{
    _transform = transform;
    _timestamp.setDirty();
}

void RenderObject::setVaryings(const sp<Varyings>& varyings)
{
    _varyings = varyings;
    _timestamp.setDirty();
}

const Box& RenderObject::tag() const
{
    return _tag;
}

void RenderObject::setTag(const Box& tag)
{
    _tag = tag;
}

const sp<Disposed>& RenderObject::disposed()
{
    return _disposed.ensure();
}

void RenderObject::setDisposed(const sp<Boolean>& disposed)
{
    _disposed.ensure()->set(disposed);
}

const sp<Visibility>& RenderObject::visible()
{
    return _visible.ensure();
}

void RenderObject::setVisible(bool visible)
{
    _visible.ensure()->set(visible);
    _timestamp.setDirty();
}

void RenderObject::setVisible(const sp<Boolean>& visible)
{
    _visible.ensure()->set(visible);
    _timestamp.setDirty();
}

void RenderObject::dispose()
{
    _disposed.ensure()->dispose();
}

void RenderObject::show()
{
    _visible.ensure()->set(true);
}

void RenderObject::hide()
{
    _visible.ensure()->set(false);
}

bool RenderObject::isDisposed() const
{
    return _type->val() < 0 || _disposed.val();
}

bool RenderObject::isVisible() const
{
    return _visible.val();
}

Renderable::State RenderObject::updateState(const RenderRequest& renderRequest)
{
    bool disposed = _disposed.update(renderRequest.timestamp()) && _disposed.val();
    bool dirty = VariableUtil::update(renderRequest.timestamp(), _visible, _type, _position, _size, _transform, _varyings, _visible) || _timestamp.update(renderRequest.timestamp());
    return Renderable::toState(disposed || _type->val() == -1, dirty, _visible.val());
}

Renderable::Snapshot RenderObject::snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, State state)
{
    if(state & Renderable::RENDERABLE_STATE_DIRTY)
        return Renderable::Snapshot(state, _type->val(), _position.val(), _size.val(), _transform->snapshot(postTranslate), _varyings->snapshot(pipelineInput, renderRequest.allocator()));
    return Renderable::Snapshot(state, _type->val(), V3(), V3(), Transform::Snapshot(), Varyings::Snapshot());
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(factory.getBuilder<Integer>(manifest, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _transform(factory.getBuilder<Transform>(manifest, Constants::Attributes::TRANSFORM)),
      _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, Constants::Attributes::VARYINGS)),
      _disposed(factory.getBuilder<Disposed>(manifest, Constants::Attributes::DISPOSED))
{
}

sp<RenderObject> RenderObject::BUILDER::build(const Scope& args)
{
    return sp<RenderObject>::make(_type->build(args), _position->build(args), _size->build(args), _transform->build(args), _varyings->build(args), nullptr, _disposed->build(args));
}

}
