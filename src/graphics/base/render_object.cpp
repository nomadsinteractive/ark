#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/epi/disposed.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"

#include "graphics/base/size.h"
#include "graphics/util/vec3_util.h"

#include "renderer/base/varyings.h"

namespace ark {

RenderObject::RenderObject(int32_t type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

RenderObject::RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _disposed(type.as<Disposed>())
{
}

const sp<Integer> RenderObject::type() const
{
    return _type;
}

float RenderObject::width() const
{
    return _size->width();
}

float RenderObject::height() const
{
    return _size->height();
}

const SafePtr<Size>& RenderObject::size()
{
    return _size;
}

const SafePtr<Transform>& RenderObject::transform() const
{
    return _transform;
}

void RenderObject::setType(int32_t type)
{
    _type->set(type);
    _disposed->set(false);
}

void RenderObject::setType(const sp<Integer>& type)
{
    _type->set(type);
    sp<Disposed> disposed = type.as<Disposed>();
    _disposed->set(disposed ? disposed->toBoolean() : sp<Boolean>::null());
}

float RenderObject::x() const
{
    return _position->val().x();
}

void RenderObject::setX(float x)
{
    Vec3Util::setX(_position, x);
}

void RenderObject::setX(const sp<Numeric>& x)
{
    Vec3Util::setX(_position, x);
}

float RenderObject::y() const
{
    return _position->val().y();
}

void RenderObject::setY(float y)
{
    Vec3Util::setY(_position, y);
}

void RenderObject::setY(const sp<Numeric>& y)
{
    Vec3Util::setY(_position, y);
}

float RenderObject::z() const
{
    return _position->val().z();
}

void RenderObject::setZ(float z)
{
    Vec3Util::setZ(_position, z);
}

void RenderObject::setZ(const sp<Numeric>& z)
{
    Vec3Util::setZ(_position, z);
}

V2 RenderObject::xy() const
{
    return _position->val();
}

V3 RenderObject::xyz() const
{
    return _position->val();
}

const SafePtr<Vec3>& RenderObject::position() const
{
    return _position;
}

void RenderObject::setPosition(const sp<Vec3>& position)
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

void RenderObject::setVaryings(const sp<Varyings>& varyings)
{
    _varyings = varyings;
}

const Box& RenderObject::tag() const
{
    return _tag;
}

void RenderObject::setTag(const Box& tag)
{
    _tag = tag;
}

const SafePtr<Disposed>& RenderObject::disposed() const
{
    return _disposed;
}

void RenderObject::dispose()
{
    _type->set(0);
    _position = nullptr;
    _size = nullptr;
    _transform = nullptr;
    _varyings = nullptr;
    _tag = Box();
    _disposed->dispose();
}

bool RenderObject::isDisposed() const
{
    return _disposed->isDisposed();
}

RenderObject::Snapshot RenderObject::snapshot(MemoryPool& memoryPool) const
{
    return Snapshot(_type->val(), _position->val(), _size->val(), _transform->snapshot(), _varyings->snapshot(memoryPool));
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(factory.ensureBuilder<Integer>(manifest, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION)),
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
    : _delegate(delegate), _disposable(factory.ensureBuilder<Disposed>(value))
{
}

sp<RenderObject> RenderObject::EXPIRED_STYLE::build(const sp<Scope>& args)
{
    return _delegate->build(args).absorb(_disposable->build(args));
}

RenderObject::Snapshot::Snapshot(int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
    : _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

}
