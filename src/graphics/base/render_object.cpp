#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/epi/disposed.h"
#include "core/epi/visibility.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
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

RenderObject::RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings, const sp<Disposed>& disposed)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _disposed(disposed)
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

void RenderObject::traverse(const Holder::Visitor& visitor)
{
    visitor(_tag);
    HolderUtil::visit(_type->delegate(), visitor);
    HolderUtil::visit(_position, visitor);
    HolderUtil::visit(_size, visitor);
    HolderUtil::visit(_transform, visitor);
    HolderUtil::visit(_varyings, visitor);
    HolderUtil::visit(_disposed->delegate(), visitor);
    HolderUtil::visit(_visible->delegate(), visitor);
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
    _disposed->set(false);
}

void RenderObject::setType(const sp<Integer>& type)
{
    _type->set(type);
    _disposed->set(type.as<Disposed>());
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

const sp<Disposed>& RenderObject::disposed() const
{
    return _disposed;
}

void RenderObject::setDisposed(const sp<Boolean>& disposed)
{
    _disposed->set(disposed);
}

const sp<Visibility>& RenderObject::visible() const
{
    return _visible;
}

void RenderObject::setVisible(const sp<Boolean>& visible)
{
    _visible->set(visible);
}

void RenderObject::dispose()
{
    _disposed->dispose();
}

void RenderObject::show()
{
    _visible->set(true);
}

void RenderObject::hide()
{
    _visible->set(false);
}

bool RenderObject::isDisposed() const
{
    return _type->val() < 0 || _disposed->val();
}

bool RenderObject::isVisible() const
{
    return _visible->val();
}

RenderObject::Snapshot RenderObject::snapshot(const PipelineInput& pipelineInput, Allocator& allocator) const
{
    return Snapshot(_type->val(), _position->val(), _size->val(), _transform->snapshot(), _varyings->snapshot(pipelineInput, allocator));
}

RenderObject::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(factory.ensureBuilder<Integer>(manifest, Constants::Attributes::TYPE)),
      _position(factory.getBuilder<Vec3>(manifest, Constants::Attributes::POSITION)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _transform(factory.getBuilder<Transform>(manifest, Constants::Attributes::TRANSFORM)),
      _varyings(factory.getConcreteClassBuilder<Varyings>(manifest, Constants::Attributes::VARYINGS)),
      _disposed(factory.getBuilder<Disposed>(manifest, Constants::Attributes::DISPOSED))
{
}

sp<RenderObject> RenderObject::BUILDER::build(const Scope& args)
{
    return sp<RenderObject>::make(_type->build(args), _position->build(args), _size->build(args), _transform->build(args), _varyings->build(args), _disposed->build(args));
}

RenderObject::Snapshot::Snapshot(int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
    : _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

}
