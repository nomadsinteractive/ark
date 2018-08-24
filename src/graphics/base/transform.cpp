#include "graphics/base/transform.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

#include "graphics/base/matrix.h"
#include "graphics/base/rotation.h"

namespace ark {

Transform::Transform(const sp<Rotation>& rotate, const sp<Vec>& scale, const sp<Vec>& translation)
    : _rotate(rotate), _scale(scale), _translation(translation)
{
}

Transform::Transform(const Transform& other)
    :  _rotate(other._rotate), _scale(other._scale), _translation(other._translation)
{
}

Transform::Snapshot Transform::snapshot() const
{
    Snapshot ss;
    ss.rotate_value = _rotate->radians();
    ss.rotate_direction = _rotate->direction()->val();
    ss.scale = _scale ? V3(_scale->val()) : V3(1.0f, 1.0f, 1.0f);
    ss.translate = _translation->val();
    return ss;
}

const sp<Rotation>& Transform::rotate()
{
    return _rotate.ensure();
}

void Transform::setRotate(const sp<Rotation>& rotate)
{
    _rotate = rotate;
}

const sp<Vec>& Transform::scale() const
{
    return _scale;
}

void Transform::setScale(const sp<Vec>& scale)
{
    _scale = scale;
}

const sp<Vec>& Transform::translation() const
{
    return _translation.ensure();
}

void Transform::setTranslation(const sp<Vec>& translation)
{
    _translation = translation;
}

Transform::Snapshot::Snapshot()
    : rotate_value(0), rotate_direction(Rotation::Z_AXIS), scale(V3(1.0f, 1.0f, 1.0f))
{
}

Matrix Transform::Snapshot::toMatrix() const
{
    Matrix matrix;
    toMatrix(matrix);
    return matrix;
}

void Transform::Snapshot::toMatrix(Matrix& matrix) const
{
    matrix.setIdentity();
    matrix.translate(translate.x(), translate.y(), translate.z());
    matrix.scale(scale.x(), scale.y(), scale.z());
    matrix.rotate(rotate_value, rotate_direction.x(), rotate_direction.y(), rotate_direction.z());
}

bool Transform::Snapshot::isFrontfaceCCW() const
{
    return Math::signEquals(scale.x(), scale.y());
}

bool Transform::Snapshot::operator ==(const Transform::Snapshot& other) const
{
    return translate == other.translate && scale == other.scale && rotate_value == other.rotate_value && rotate_direction == other.rotate_direction;
}

bool Transform::Snapshot::operator !=(const Transform::Snapshot& other) const
{
    return !(*this == other);
}

void Transform::Snapshot::map(float x, float y, float tx, float ty, float& mx, float& my) const
{
    if(rotate_value == 0.0f)
    {
        mx = x * scale.x() + translate.x();
        my = y * scale.y() + translate.y();
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
        x = p.x() * scale.x() + translate.x();
        y = p.y() * scale.y() + translate.y();
        z = p.z() * scale.z() + translate.z();
    }
    else
    {
        const Matrix matrix = toMatrix();
        matrix.map(p.x(), p.y(), p.z(), x, y, z);
    }
    return V3(x, y, z);
}

Transform::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _rotate(factory.getConcreteClassBuilder<Rotation>(manifest, Constants::Attributes::ROTATE)), _scale(factory.getBuilder<Vec>(manifest, "scale")),
      _translation(factory.getBuilder<Vec>(manifest, Constants::Attributes::TRANSLATION))
{
}

sp<Transform> Transform::BUILDER::build(const sp<Scope>& args)
{
    return sp<Transform>::make(_rotate->build(args), _scale->build(args), _translation->build(args));
}

Transform::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _impl(parent, Documents::fromProperties(Strings::parseProperties(value)))
{
}

sp<Transform> Transform::DICTIONARY::build(const sp<Scope>& args)
{
    return _impl.build(args);
}

template<> ARK_API const sp<Transform> Null::ptr()
{
    return Ark::instance().obtain<Transform>();
}

}
