#include "graphics/base/transform.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/null.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"
#include "core/util/holder_util.h"
#include "core/util/updatable_util.h"

#include "graphics/base/v3.h"
#include "graphics/impl/transform/transform_none.h"
#include "graphics/impl/transform/transform_linear_2d.h"
#include "graphics/impl/transform/transform_simple_2d.h"
#include "graphics/impl/transform/transform_linear_3d.h"
#include "graphics/impl/transform/transform_simple_3d.h"
#include "graphics/util/matrix_util.h"

namespace ark {

namespace {

class TransformMat4 : public Mat4 {
public:
    TransformMat4(sp<Transform> transform)
        : _transform(std::move(transform)), _value(_transform->snapshot(V3(0)).toMatrix()) {
    }

    virtual bool update(uint64_t timestamp) override {
        if(_transform->update(timestamp)) {
            _value = _transform->snapshot(V3(0)).toMatrix();
            return true;
        }
        return false;
    }

    virtual M4 val() override {
        return _value;
    }

private:
    sp<Transform> _transform;
    M4 _value;
};

}

Transform::Transform(Type type, sp<Rotation> rotate, sp<Vec3> scale, sp<Vec3> translation)
    : _type(type), _rotation(std::move(rotate), V4(0, 0, 0, 1.0f)), _scale(std::move(scale), V3(1.0f)), _translation(std::move(translation)), _delegate(makeDelegate())
{
}

Transform::Transform(sp<Transform::Delegate> delegate)
    : _type(TYPE_DELEGATED), _delegate(std::move(delegate))
{
}

Transform::Snapshot Transform::snapshot(const V3& postTranslate) const
{
    return Snapshot(*this, postTranslate);
}

bool Transform::update(uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, _rotation, _scale, _translation) || _timestamp.update(timestamp);
}

const sp<Rotation>& Transform::rotation()
{
    return tryUpdateDelegate(_rotation);
}

void Transform::setRotation(sp<Rotation> rotation)
{
    _rotation = std::move(rotation);
    doUpdateDelegate();
}

const sp<Vec3>& Transform::scale()
{
    return tryUpdateDelegate(_scale);
}

void Transform::setScale(sp<Vec3> scale)
{
    _scale = std::move(scale);
    doUpdateDelegate();
}

const sp<Vec3>& Transform::translation()
{
    return tryUpdateDelegate(_translation);
}

void Transform::setTranslation(sp<Vec3> translation)
{
    _translation = std::move(translation);
    doUpdateDelegate();
}

sp<Mat4> Transform::toMatrix(sp<Transform> self)
{
    return sp<TransformMat4>::make(std::move(self));
}

void Transform::doUpdateDelegate()
{
    _delegate = makeDelegate();
    _timestamp.markDirty();
}

sp<Transform::Delegate> Transform::makeDelegate() const
{
    DCHECK(_type != TYPE_DELEGATED, "Delegated Transform may not be updated");

    if(!_rotation && !_scale && !_translation)
        return Null::toSafePtr<Transform::Delegate>(nullptr);

    return _rotation ? makeTransformLinear() : makeTransformSimple();
}

sp<Transform::Delegate> Transform::makeTransformLinear() const
{
    return _type == TYPE_LINEAR_2D ? Global<TransformLinear2D>().cast<Transform::Delegate>() : Global<TransformLinear3D>().cast<Transform::Delegate>();
}

sp<Transform::Delegate> Transform::makeTransformSimple() const
{
    return _type == TYPE_LINEAR_2D ? Global<TransformSimple2D>().cast<Transform::Delegate>() : Global<TransformSimple3D>().cast<Transform::Delegate>();
}

Transform::Snapshot::Snapshot(const Transform& transform, const V3& postTranslate)
    : _delegate(transform._delegate)
{
    _delegate->snapshot(transform, postTranslate, *this);
}

M4 Transform::Snapshot::toMatrix() const
{
    return _delegate->toMatrix(*this);
}

V3 Transform::Snapshot::transform(const V3& p) const
{
    return _delegate->transform(*this, p);
}

template<> ARK_API sp<Transform> Null::safePtr()
{
    return sp<Transform>::make();
}

template<> ARK_API Transform::Type StringConvert::eval<Transform::Type>(const String& str)
{
    if(str == "2d")
        return Transform::TYPE_LINEAR_2D;
    DCHECK(str == "3d", "Unknow transform type: %s", str.c_str());
    return Transform::TYPE_LINEAR_3D;
}

}
