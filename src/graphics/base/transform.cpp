#include "graphics/base/transform.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

#include "graphics/base/matrix.h"

namespace ark {

Transform::Transform(const sp<Numeric>& rotate, const sp<VV>& scale, const sp<VV>& translation)
    : _rotation(rotate), _scale(scale ? scale : identity()), _translation(translation)
{
}

Transform::Transform(const Transform& other)
    :  _rotation(other._rotation), _scale(other._scale), _translation(other._translation)
{
}

Transform::Snapshot Transform::snapshot() const
{
    Snapshot ss;
    ss.rotation = _rotation->val();
    ss.scale = _scale->val();
    ss.translate = _translation->val();
    return ss;
}

const sp<Numeric>& Transform::rotation()
{
    return _rotation.ensure();
}

void Transform::setRotation(const sp<Numeric>& rotate)
{
    _rotation = rotate;
}

const sp<VV>& Transform::scale() const
{
    return _scale;
}

void Transform::setScale(const sp<VV>& scale)
{
    _scale = scale;
}

const sp<VV>& Transform::translation() const
{
    return _translation.ensure();
}

void Transform::setTranslation(const sp<VV>& translation)
{
    _translation = translation;
}

const sp<VV>& Transform::identity()
{
    static sp<VV> IDENTITY = sp<VV::Const>::make(V::identity());
    return IDENTITY;
}

Transform::Snapshot::Snapshot()
    : rotation(0), scale(V::identity())
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
    matrix.rotate(rotation, 0, 0, 1.0f);
}

bool Transform::Snapshot::isFrontfaceCCW() const
{
    return Math::signEquals(scale.x(), scale.y());
}

bool Transform::Snapshot::operator ==(const Transform::Snapshot& other) const
{
    return translate == other.translate && scale == other.scale && rotation == other.rotation;
}

bool Transform::Snapshot::operator !=(const Transform::Snapshot& other) const
{
    return !(*this == other);
}

void Transform::Snapshot::map(float x, float y, float tx, float ty, float& mx, float& my) const
{
    if(rotation == 0.0f)
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
    if(rotation == 0.0f)
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

Transform::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _rotation(parent.getBuilder<Numeric>(doc, "rotation")), _scale(parent.getBuilder<VV>(doc, "scale")), _translation(parent.getBuilder<VV>(doc, Constants::Attributes::TRANSLATION))
{
}

sp<Transform> Transform::BUILDER::build(const sp<Scope>& args)
{
    return sp<Transform>::make(_rotation->build(args), _scale->build(args), _translation->build(args));
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
