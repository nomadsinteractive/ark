#include "graphics/base/transform.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_dirty.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/null.h"
#include "core/util/string_convert.h"

#include "graphics/base/v3.h"
#include "graphics/impl/transform/transform_none.h"
#include "graphics/impl/transform/transform_linear_2d.h"
#include "graphics/impl/transform/transform_simple_2d.h"
#include "graphics/impl/transform/transform_linear_3d.h"
#include "graphics/impl/transform/transform_simple_3d.h"

namespace ark {

namespace {

class TransformDelegateMat4 final : public Transform::Delegate {
public:
    TransformDelegateMat4(sp<Mat4> matrix)
        : _matrix(std::move(matrix)) {
    }

    bool update(const Transform::Stub& transform, uint64_t timestamp) override
    {
        return _matrix->update(timestamp);
    }

    void snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const override
    {
        snapshot.makeData<M4>() = _matrix->val();
    }

    V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override
    {
        return MatrixUtil::mul(snapshot.getData<M4>(), position);
    }

    M4 toMatrix(const Transform::Snapshot& snapshot) const override
    {
        return snapshot.getData<M4>();
    }

private:
    sp<Mat4> _matrix;
};

}

class Transform::TransformToMat4 final : public Mat4 {
public:
    TransformToMat4(const Transform& transform)
        : _transform(transform), _value(_transform.snapshot().toMatrix()) {
    }

    bool update(uint64_t timestamp) override {
        if(_transform._delegate->update(_transform._stub, timestamp)) {
            _value = _transform.snapshot().toMatrix();
            return true;
        }
        return false;
    }

    M4 val() override {
        return _value;
    }

private:
    Transform _transform;
    M4 _value;
};

Transform::Transform(Type type, sp<Rotation> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : _type(type), _stub(sp<Stub>::make(Stub{{std::move(rotation), constants::QUATERNION_ZERO}, {std::move(scale), V3(1.0f)}, {std::move(translation)}}))
{
    doUpdateDelegate();
}

Transform::Transform(sp<Transform::Delegate> delegate)
    : _type(TYPE_DELEGATED), _stub(sp<Stub>::make(Stub{{nullptr, constants::QUATERNION_ZERO}, {nullptr, V3(1.0f)}, {}})), _delegate(std::move(delegate))
{
    doUpdateDelegate();
}

Transform::Snapshot Transform::snapshot() const
{
    return Snapshot(*this);
}

bool Transform::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

M4 Transform::val()
{
    return _wrapped->val();
}

const sp<Rotation>& Transform::rotation()
{
    return tryUpdateDelegate(_stub->_rotation);
}

void Transform::setRotation(sp<Rotation> rotation)
{
    _stub->_rotation.reset(std::move(rotation));
    doUpdateDelegate();
}

const sp<Vec3>& Transform::scale()
{
    return tryUpdateDelegate(_stub->_scale);
}

void Transform::setScale(sp<Vec3> scale)
{
    _stub->_scale.reset(std::move(scale));
    doUpdateDelegate();
}

const sp<Vec3>& Transform::translation()
{
    return tryUpdateDelegate(_stub->_translation);
}

void Transform::setTranslation(sp<Vec3> translation)
{
    _stub->_translation.reset(std::move(translation));
    doUpdateDelegate();
}

void Transform::reset(sp<Mat4> transform)
{
    _type = TYPE_DELEGATED;
    _delegate = sp<Delegate>::make<TransformDelegateMat4>(std::move(transform));
    doUpdateDelegate();
}

void Transform::doUpdateDelegate()
{
    if(_type != TYPE_DELEGATED)
        _delegate = makeDelegate();
    _wrapped = sp<Mat4>::make<VariableDirty<M4>>(sp<Mat4>::make<TransformToMat4>(*this), *this);
}

sp<Transform::Delegate> Transform::makeDelegate() const
{
    CHECK(_type != TYPE_DELEGATED, "Delegated Transform may not be updated");

    if(!_stub->_rotation && !_stub->_scale && !_stub->_translation)
        return Global<TransformNone>().cast<Delegate>();

    return _stub->_rotation ? makeTransformLinear() : makeTransformSimple();
}

sp<Transform::Delegate> Transform::makeTransformLinear() const
{
    return _type == TYPE_LINEAR_2D ? Global<TransformLinear2D>().cast<Transform::Delegate>() : Global<TransformLinear3D>().cast<Transform::Delegate>();
}

sp<Transform::Delegate> Transform::makeTransformSimple() const
{
    return _type == TYPE_LINEAR_2D ? Global<TransformSimple2D>().cast<Transform::Delegate>() : Global<TransformSimple3D>().cast<Transform::Delegate>();
}

Transform::Snapshot::Snapshot(const Transform& transform)
    : _delegate(transform._delegate)
{
    _delegate->snapshot(transform._stub, *this);
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
