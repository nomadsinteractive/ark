#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/iterator.h"
#include "core/inf/variable.h"
#include "core/epi/lifecycle.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"

#include "graphics/base/size.h"
#include "graphics/util/vec2_util.h"

#include "renderer/base/varyings.h"

namespace ark {

RenderObject::RenderObject(int32_t type, const sp<Vec>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings)
    : _type(sp<IntegerWrapper>::make(type)), _position(Null::toSafe<Vec>(position)), _size(size), _transform(transform), _varyings(Null::toSafe<Varyings>(varyings))
{
}

RenderObject::RenderObject(const sp<Integer>& type, const sp<Vec>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings)
    : _type(sp<IntegerWrapper>::make(type)), _position(Null::toSafe<Vec>(position)), _size(size), _transform(transform), _varyings(Null::toSafe<Varyings>(varyings)), _type_expired(type.as<Lifecycle>())
{
}

const sp<Integer> RenderObject::type() const
{
    return _type;
}

const sp<Varyings>& RenderObject::varyings() const
{
    return _varyings;
}

float RenderObject::width() const
{
    return _size->width();
}

float RenderObject::height() const
{
    return _size->height();
}

const sp<Size>& RenderObject::size()
{
    return _size.ensure();
}

const sp<Transform>& RenderObject::transform() const
{
    return _transform.ensure();
}

void RenderObject::setType(int32_t type)
{
    _type->set(type);
    _type_expired = nullptr;
}

void RenderObject::setType(const sp<Integer>& type)
{
    _type->set(type);
    _type_expired = type.as<Lifecycle>();
}

float RenderObject::x() const
{
    return _position->val().x();
}

void RenderObject::setX(float x)
{
    VecUtil::setX(_position, x);
}

void RenderObject::setX(const sp<Numeric>& x)
{
    VecUtil::setX(_position, x);
}

float RenderObject::y() const
{
    return _position->val().y();
}

void RenderObject::setY(float y)
{
    VecUtil::setY(_position, y);
}

void RenderObject::setY(const sp<Numeric>& y)
{
    VecUtil::setY(_position, y);
}

V2 RenderObject::xy() const
{
    return _position->val();
}

const sp<Vec>& RenderObject::position() const
{
    return _position;
}

void RenderObject::setPosition(const sp<Vec>& position)
{
    _position = position;
}

void RenderObject::setSize(const sp<Size>& size)
{
    _size = size;
}

void RenderObject::setTransform(const sp<Transform>& transform)
{
    _transform = transform;
}

void RenderObject::setVaryings(const sp<Varyings>& filter)
{
    _varyings = Null::toSafe<Varyings>(filter);
}

const Box& RenderObject::tag() const
{
    return _tag;
}

void RenderObject::setTag(const Box& tag)
{
    _tag = tag;
}

bool RenderObject::isExpired() const
{
    return _type_expired && _type_expired->val();
}

RenderObject::Snapshot RenderObject::snapshot(MemoryPool& memoryPool) const
{
    return Snapshot(_type->val(), _position->val(), _size->val(), _transform->snapshot(), _varyings->snapshot(memoryPool));
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(factory.ensureBuilder<Integer>(manifest, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec>(manifest, Constants::Attributes::POSITION)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _transform(factory.getBuilder<Transform>(manifest, Constants::Attributes::TRANSFORM)),
      _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, Constants::Attributes::VARYINGS))
{
}

sp<RenderObject> RenderObject::BUILDER::build(const sp<Scope>& args)
{
    const sp<Integer> type = _type->build(args);
    return sp<RenderObject>::make(type, _position->build(args), _size->build(args), _transform->build(args), _varyings->build(args));
}

RenderObject::EXPIRED_STYLE::EXPIRED_STYLE(BeanFactory& factory, const sp<Builder<RenderObject>>& delegate, const String& value)
    : _delegate(delegate), _expired(factory.ensureBuilder<Lifecycle>(value))
{
}

sp<RenderObject> RenderObject::EXPIRED_STYLE::build(const sp<Scope>& args)
{
    return _delegate->build(args).absorb(_expired->build(args));
}

RenderObject::Snapshot::Snapshot(int32_t type, const V& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
    : _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

}
