#include "graphics/base/transform.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

#include "graphics/base/matrix.h"

namespace ark {

Transform::Transform(const sp<VV>& pivot, const sp<Numeric>& rotate, const sp<VV>& scale, const sp<VV>& translation)
    : _pivot(pivot), _rotation(rotate), _scale(scale ? scale : sp<VV>::adopt(new VV::Impl(V(1.0f, 1.0f)))), _translation(translation)
{
}

Transform::Transform(const Transform& other)
    :  _pivot(other._pivot), _rotation(other._rotation), _scale(other._scale), _translation(other._translation)
{
}

//Transform::Transform()
//    : _pivot(nullptr), _scale(sp<VV2::Impl>::make(V2(1.0f, 1.0f)))
//{
//}

Transform::Snapshot Transform::snapshot(float px, float py) const
{
    Snapshot ss(px, py);
    if(_pivot)
    {
        const V p = _pivot->val();
        ss.pivot_x += p.x();
        ss.pivot_y += p.y();
    }
    ss.rotation = _rotation->val();
    ss.scale = _scale->val();
    ss.translate = _translation->val();
    return ss;
}

const sp<VV>& Transform::pivot() const
{
    return _pivot;
}

void Transform::setPivot(const sp<VV>& pivot)
{
    _pivot = pivot;
}

const sp<Numeric>& Transform::rotation()
{
    return _rotation.ensure();
}

void Transform::setRotation(const sp<Numeric>& rotate)
{
    _rotation.assign(rotate);
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
    _translation.assign(translation);
}

Transform::Snapshot::Snapshot(float px, float py)
    : pivot_x(px), pivot_y(py), rotation(0), scale(1.0f, 1.0f)
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
    matrix.translate(translate.x(), translate.y(), 0);
    matrix.scale(scale.x(), scale.y(), 1.0f);
    matrix.rotate(rotation, 0, 0, 1.0f);
    matrix.translate(-pivot_x, -pivot_y, 0);
}

bool Transform::Snapshot::isFrontfaceCCW() const
{
    return Math::signEquals(scale.x(), scale.y());
}

bool Transform::Snapshot::operator ==(const Transform::Snapshot& other) const
{
    return translate == other.translate && scale == other.scale
            && rotation == other.rotation && pivot_x == other.pivot_x && pivot_y == other.pivot_y;
}

bool Transform::Snapshot::operator !=(const Transform::Snapshot& other) const
{
    return !(*this == other);
}

void Transform::Snapshot::map(float x, float y, float tx, float ty, float& mx, float& my) const
{
    if(rotation == 0.0f)
    {
        mx = (x - pivot_x) * scale.x() + translate.x();
        my = (y - pivot_y) * scale.y() + translate.y();
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

Transform::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _pivot(parent.getBuilder<VV>(doc, "pivot")), _rotation(parent.getBuilder<Numeric>(doc, "rotation")), _scale(parent.getBuilder<VV>(doc, "scale")), _translation(parent.getBuilder<VV>(doc, "translation"))
{
}

sp<Transform> Transform::BUILDER::build(const sp<Scope>& args)
{
    return sp<Transform>::make(_pivot->build(args), _rotation->build(args), _scale->build(args), _translation->build(args));
}

Transform::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _impl(parent, Documents::fromProperties(Strings::parseProperties(value)))
{
}

sp<Transform> Transform::DICTIONARY::build(const sp<Scope>& args)
{
    return _impl.build(args);
}

template<> ARK_API const sp<Transform>& Null::ptr()
{
    static const sp<Transform> inst = sp<Transform>::make(Null::ptr<VV>(), Null::ptr<Numeric>(), sp<VV::Const>::make(V(1.0f, 1.0f)), Null::ptr<VV>());
    return inst;
}

}
