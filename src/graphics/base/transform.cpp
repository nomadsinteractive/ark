#include "graphics/base/transform.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

#include "graphics/base/v3.h"
#include "graphics/impl/transform/transform_none.h"
#include "graphics/impl/transform/transform_linear_2d.h"
#include "graphics/impl/transform/transform_simple_2d.h"
#include "graphics/impl/transform/transform_linear_3d.h"
#include "graphics/impl/transform/transform_simple_3d.h"
#include "graphics/util/matrix_util.h"

namespace ark {

Transform::Transform(Type type, const sp<Rotate>& rotate, const sp<Vec3>& scale, const sp<Vec3>& translate)
    : _type(type), _rotate(rotate), _scale(scale, V3(1.0f)), _pivot(translate), _delegate(makeDelegate())
{
}

Transform::Transform(sp<Transform::Delegate> delegate)
    : _type(TYPE_DELEGATED), _delegate(std::move(delegate))
{
}

void Transform::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_rotate.delegate(), visitor);
    HolderUtil::visit(_scale.delegate(), visitor);
    HolderUtil::visit(_pivot.delegate(), visitor);
}

Transform::Snapshot Transform::snapshot() const
{
    return Snapshot(*this);
}

bool Transform::update(uint64_t timestamp) const
{
    return VariableUtil::update(timestamp, _rotate, _scale, _pivot);
}

const sp<Rotate>& Transform::rotate()
{
    return _rotate.ensure<DelegateUpdater>(DelegateUpdater(*this));
}

void Transform::setRotate(const sp<Rotate>& rotate)
{
    _rotate = rotate;
    updateDelegate();
}

const sp<Vec3>& Transform::scale()
{
    return _scale.ensure<DelegateUpdater>(DelegateUpdater(*this));
}

void Transform::setScale(const sp<Vec3>& scale)
{
    _scale = scale;
    updateDelegate();
}

const sp<Vec3>& Transform::pivot()
{
    return _pivot.ensure<DelegateUpdater>(DelegateUpdater(*this));
}

void Transform::setPivot(const sp<Vec3>& pivot)
{
    _pivot = pivot;
    updateDelegate();
}

void Transform::updateDelegate()
{
    _delegate = makeDelegate();
}

sp<Transform::Delegate> Transform::makeDelegate() const
{
    DCHECK(_type != TYPE_DELEGATED, "Delegated Transform may not be updated");

    if(!_rotate && !_scale && !_pivot)
        return Null::toSafe<Transform::Delegate>(nullptr);

    return _rotate ? makeTransformLinear() : makeTransformSimple();
}

sp<Transform::Delegate> Transform::makeTransformLinear() const
{
    return _type == TYPE_LINEAR_2D ? sp<Transform::Delegate>::make<TransformLinear2D>() : sp<Transform::Delegate>::make<TransformLinear3D>();
}

sp<Transform::Delegate> Transform::makeTransformSimple() const
{
    return _type == TYPE_LINEAR_2D ? sp<Transform::Delegate>::make<TransformSimple2D>() : sp<Transform::Delegate>::make<TransformSimple2D>();
}

Transform::Snapshot::Snapshot(const Transform& transform)
    : _delegate(transform._delegate)
{
    _delegate->snapshot(transform, *this);
}

M4 Transform::Snapshot::toMatrix() const
{
    return _delegate->toMatrix(*this);
}

V3 Transform::Snapshot::transform(const V3& p) const
{
    return _delegate->transform(*this, p);
}

Transform::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _type(Documents::getAttribute(manifest, Constants::Attributes::TYPE, Transform::TYPE_LINEAR_3D)),
      _rotate(factory.getBuilder<Rotate>(manifest, Constants::Attributes::ROTATE)), _scale(factory.getBuilder<Vec3>(manifest, "scale")),
      _pivot(factory.getBuilder<Vec3>(manifest, "pivot"))
{
}

sp<Transform> Transform::BUILDER::build(const Scope& args)
{
    return sp<Transform>::make(_type, _rotate->build(args), _scale->build(args), _pivot->build(args));
}

Transform::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _impl(factory, Documents::fromProperties(Strings::parseProperties(value)))
{
}

sp<Transform> Transform::DICTIONARY::build(const Scope& args)
{
    return _impl.build(args);
}

template<> ARK_API sp<Transform> Null::ptr()
{
    return sp<Transform>::make();
}

template<> ARK_API Transform::Type Conversions::to<String, Transform::Type>(const String& str)
{
    if(str == "2d")
        return Transform::TYPE_LINEAR_2D;
    DCHECK(str == "3d", "Unknow transform type: %s", str.c_str());
    return Transform::TYPE_LINEAR_3D;
}

}
