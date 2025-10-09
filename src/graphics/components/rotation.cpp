#include "graphics/components/rotation.h"

#include <glm/gtx/quaternion.hpp>

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_dirty_mark.h"
#include "core/impl/variable/variable_wrapper.h"

#include "graphics/base/rotation_axis_theta.h"
#include "graphics/base/rotation_euler.h"
#include "graphics/base/mat.h"

namespace ark {

namespace {

class Mat4Quaternion final : public Mat4 {
public:
    Mat4Quaternion(sp<Vec4> quaternion)
        : _quaternion(std::move(quaternion)) {
        update(Timestamp::now());
    }

    bool update(const uint64_t timestamp) override
    {
        if(_quaternion->update(timestamp))
        {
            const V4 quaternion = _quaternion->val();
            _matrix = {glm::toMat4(glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z()))};
            return true;
        }
        return false;
    }

    M4 val() override
    {
        return _matrix;
    }

private:
    sp<Vec4> _quaternion;
    M4 _matrix;
};

}

Rotation::Rotation(const V4 quaternion)
    : Rotation(sp<Vec4>::make<Vec4::Const>(quaternion))
{
}

Rotation::Rotation(sp<Vec4> quaternion)
    : Rotation(sp<Vec4Wrapper>::make(std::move(quaternion)))
{
}

Rotation::Rotation(const sp<Vec4Wrapper>& vec4Wrapper)
    : _delegate(vec4Wrapper), _wrapper(vec4Wrapper)
{
}

Rotation::Rotation(sp<Vec4> delegate, sp<Wrapper<Vec4>> wrapper)
    : _delegate(std::move(delegate)), _wrapper(std::move(wrapper))
{
}

V4 Rotation::val()
{
    return _delegate->val();
}

bool Rotation::update(const uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void Rotation::reset(sp<Vec4> quaternion)
{
    VariableDirtyMark<V4>::markDirty(_wrapper, std::move(quaternion));
}

void Rotation::setRotation(const V3 axis, float theta)
{
    setRotation(sp<Vec3>::make<Vec3::Const>(axis), sp<Numeric>::make<Numeric::Const>(theta));
}

void Rotation::setRotation(sp<Vec3> axis, sp<Numeric> theta)
{
    reset(sp<Vec4>::make<RotationAxisTheta>(std::move(axis), std::move(theta)));
}

sp<RotationAxisTheta> Rotation::getAxisTheta() const
{
    return _delegate.asInstance<RotationAxisTheta>();
}

void Rotation::setEuler(float pitch, float yaw, float roll)
{
    setEuler(sp<Numeric::Const>::make(pitch), sp<Numeric::Const>::make(yaw), sp<Numeric::Const>::make(roll));
}

void Rotation::setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
{
    reset(sp<Vec4>::make<RotationEuler>(std::move(pitch), std::move(yaw), std::move(roll)));
}

sp<RotationEuler> Rotation::getEuler() const
{
    return _delegate.asInstance<RotationEuler>();
}

sp<Mat4> Rotation::toMatrix() const
{
    return sp<Mat4>::make<Mat4Quaternion>(_delegate);
}

}
