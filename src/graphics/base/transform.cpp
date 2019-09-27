#include "graphics/base/transform.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/holder_util.h"

#include "graphics/base/matrix.h"
#include "graphics/base/rotate.h"
#include "graphics/impl/vec/vec3_impl.h"

namespace ark {

Transform::Transform(const sp<Rotate>& rotate, const sp<Vec3>& scale, const sp<Vec3>& translate)
    : _rotate(rotate), _scale(scale ? scale : Ark::instance().obtain<Vec3Impl>(1.0f, 1.0f, 1.0f).cast<Vec3>()), _pivot(translate)
{
}

void Transform::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_rotate, visitor);
    HolderUtil::visit(_scale, visitor);
    HolderUtil::visit(_pivot, visitor);
}

Transform::Snapshot Transform::snapshot() const
{
    Snapshot ss;
    ss.rotate_value = _rotate->rotation();
    ss.rotate_direction = _rotate->direction()->val();
    ss.scale = _scale->val();
    ss.pivot = _pivot->val();
    return ss;
}

const sp<Rotate>& Transform::rotate()
{
    return _rotate;
}

void Transform::setRotate(const sp<Rotate>& rotate)
{
    _rotate = rotate;
}

const sp<Vec3>& Transform::scale() const
{
    return _scale;
}

void Transform::setScale(const sp<Vec3>& scale)
{
    _scale = scale;
}

const sp<Vec3>& Transform::pivot() const
{
    return _pivot;
}

void Transform::setPivot(const sp<Vec3>& pivot)
{
    _pivot = pivot;
}

Transform::Snapshot::Snapshot()
    : rotate_value(0), rotate_direction(Rotate::Z_AXIS), scale(V3(1.0f, 1.0f, 1.0f))
{
}

Matrix Transform::Snapshot::toMatrix() const
{
    Matrix matrix;
    matrix.rotate(rotate_value, rotate_direction.x(), rotate_direction.y(), rotate_direction.z());
    matrix.scale(scale.x(), scale.y(), scale.z());
    matrix.translate(pivot.x(), pivot.y(), pivot.z());
    return matrix;
}

bool Transform::Snapshot::operator ==(const Transform::Snapshot& other) const
{
    return pivot == other.pivot && scale == other.scale && rotate_value == other.rotate_value && rotate_direction == other.rotate_direction;
}

bool Transform::Snapshot::operator !=(const Transform::Snapshot& other) const
{
    return !(*this == other);
}

void Transform::Snapshot::map(float x, float y, float tx, float ty, float& mx, float& my) const
{
    if(rotate_value == 0.0f)
    {
        mx = (x + pivot.x()) * scale.x();
        my = (y + pivot.y()) * scale.y();
    }
    else
    {
        Matrix matrix = toMatrix();
        float mz;
        matrix.map(x, y, 0.0f, mx, my, mz);
    }
    mx += tx;
    my += ty;
}

V3 Transform::Snapshot::mapXYZ(const V3& p) const
{
    float x, y, z;
    if(rotate_value == 0.0f)
    {
        x = (p.x() + pivot.x()) * scale.x();
        y = (p.y() + pivot.y()) * scale.y();
        z = (p.z() + pivot.z()) * scale.z();
    }
    else
    {
        const Matrix matrix = toMatrix();
        matrix.map(p.x(), p.y(), p.z(), x, y, z);
    }
    return V3(x, y, z);
}

Transform::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _rotate(factory.getBuilder<Rotate>(manifest, Constants::Attributes::ROTATE)), _scale(factory.getBuilder<Vec3>(manifest, "scale")),
      _pivot(factory.getBuilder<Vec3>(manifest, "pivot"))
{
}

sp<Transform> Transform::BUILDER::build(const Scope& args)
{
    return sp<Transform>::make(_rotate->build(args), _scale->build(args), _pivot->build(args));
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
    return Ark::instance().obtain<Transform>();
}

}
