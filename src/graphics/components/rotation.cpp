#include "graphics/components/rotation.h"

#include <glm/gtx/quaternion.hpp>

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_dirty_mark.h"

#include "graphics/base/rotation_axis_theta.h"
#include "graphics/base/rotation_euler.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class Mat4Quaternion final : public Mat4 {
public:
    Mat4Quaternion(sp<Vec4> quaternion)
        : _quaternion(std::move(quaternion)) {
        doUpdate();
    }

    bool update(const uint64_t timestamp) override
    {
        if(_quaternion->update(timestamp))
        {
            doUpdate();
            return true;
        }
        return false;
    }

    M4 val() override
    {
        return _matrix;
    }

private:
    void doUpdate()
    {
        const V4 quaternion = _quaternion->val();
        _matrix = {glm::toMat4(glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z()))};
    }

private:
    sp<Vec4> _quaternion;
    M4 _matrix;
};

}

Rotation::Rotation(const V4& quaternion)
    : Rotation(sp<Vec4>::make<Vec4::Const>(quaternion))
{
}

Rotation::Rotation(sp<Vec4> quaternion)
    : Wrapper(sp<Vec4>::make<VariableDirtyMark<V4>>(std::move(quaternion), *this))
{
}

V4 Rotation::val()
{
    return _wrapped->val();
}

bool Rotation::update(const uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

void Rotation::reset(sp<Vec4> quaternion)
{
    _wrapped = sp<Vec4>::make<VariableDirtyMark<V4>>(std::move(quaternion), *this);
}

void Rotation::setRotation(const V3& axis, float theta)
{
    setRotation(sp<Vec3>::make<Vec3::Const>(axis), sp<Numeric>::make<Numeric::Const>(theta));
}

void Rotation::setRotation(sp<Vec3> axis, sp<Numeric> theta)
{
    _wrapped = sp<Vec4>::make<VariableDirtyMark<V4>>(sp<Vec4>::make<RotationAxisTheta>(std::move(axis), std::move(theta)), *this);
}

sp<RotationAxisTheta> Rotation::getAxisTheta() const
{
    return _wrapped.asInstance<RotationAxisTheta>();
}

void Rotation::setEuler(float pitch, float yaw, float roll)
{
    setEuler(sp<Numeric::Const>::make(pitch), sp<Numeric::Const>::make(yaw), sp<Numeric::Const>::make(roll));
}

void Rotation::setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
{
    _wrapped = sp<Vec4>::make<VariableDirtyMark<V4>>(sp<Vec4>::make<RotationEuler>(std::move(pitch), std::move(yaw), std::move(roll)), *this);
}

sp<RotationEuler> Rotation::getEuler() const
{
    return _wrapped.asInstance<RotationEuler>();
}

sp<Mat4> Rotation::toMatrix(sp<Vec4> self)
{
    return sp<Mat4>::make<Mat4Quaternion>(std::move(self));
}

}
