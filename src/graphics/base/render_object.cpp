#include "graphics/base/render_object.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/holder_util.h"
#include "core/util/numeric_util.h"
#include "core/util/variable_util.h"

#include "graphics/util/vec3_util.h"

#include "renderer/base/varyings.h"

namespace ark {

RenderObject::RenderObject(int32_t type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _disposed(nullptr, false), _visible(nullptr, true)
{
}

RenderObject::RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _disposed(type.as<Disposed>(), false), _visible(nullptr, true)
{
}

RenderObject::RenderObject(const sp<Integer>& type, const sp<Vec3>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Varyings>& varyings, const sp<Disposed>& disposed)
    : _type(sp<IntegerWrapper>::make(type)), _position(position), _size(size), _transform(transform), _varyings(varyings), _disposed(disposed, false), _visible(nullptr, true)
{
}

const sp<Integer> RenderObject::type() const
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
    _disposed = nullptr;
}

void RenderObject::setType(const sp<Integer>& type)
{
    _type->set(type);
    _disposed = type.as<Disposed>();
}

float RenderObject::x() const
{
    return _position.val().x();
}

void RenderObject::setX(float x)
{
    Vec3Util::setX(_position.ensure(), x);
}

void RenderObject::setX(const sp<Numeric>& x)
{
    Vec3Util::setX(_position.ensure(), x);
}

float RenderObject::y() const
{
    return _position.val().y();
}

void RenderObject::setY(float y)
{
    Vec3Util::setY(_position.ensure(), y);
}

void RenderObject::setY(const sp<Numeric>& y)
{
    Vec3Util::setY(_position.ensure(), y);
}

float RenderObject::z() const
{
    return _position.val().z();
}

void RenderObject::setZ(float z)
{
    Vec3Util::setZ(_position.ensure(), z);
}

void RenderObject::setZ(const sp<Numeric>& z)
{
    Vec3Util::setZ(_position.ensure(), z);
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

void RenderObject::setVisible(const sp<Boolean>& visible)
{
    _visible.ensure()->set(visible);
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

Renderable::Snapshot RenderObject::snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest)
{
    if(_disposed.update(renderRequest.timestamp()) && _disposed.val())
        return Renderable::Snapshot();

    bool dirty = VariableUtil::update(renderRequest.timestamp(), _visible, _type, _position, _size, _transform, _varyings, _visible);
    return Renderable::Snapshot(false, dirty, _visible.val(), _type->val(), _position.val(), _size.val(), _transform->snapshot(), _varyings->snapshot(pipelineInput, renderRequest.allocator()));
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

}
