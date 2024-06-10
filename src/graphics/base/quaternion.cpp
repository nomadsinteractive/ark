#include "graphics/base/quaternion.h"

#include <glm/gtx/quaternion.hpp>

#include "core/ark.h"
#include "core/base/constants.h"
#include "core/impl/variable/variable_dirty.h"
#include "core/util/updatable_util.h"
#include "core/util/math.h"

#include "graphics/base/v3.h"
#include "graphics/util/vec3_type.h"

namespace ark {

namespace {

class AxisRotation : public Vec4 {
public:
    AxisRotation(sp<Numeric> theta, sp<Vec3> axis)
        : _theta(std::move(theta)), _axis(axis ? Vec3Type::normalize(std::move(axis)) : sp<Vec3>::make<Vec3::Const>(constants::AXIS_Z)), _val(updateQuaternion()) {
    }

    bool update(uint64_t timestamp) override {
        if(UpdatableUtil::update(timestamp, _theta, _axis)) {
            _val = updateQuaternion();
            return true;
        }
        return false;
    }

    V4 val() override {
        return _val;
    }

private:
    V4 updateQuaternion() const {
        const V3 axis = _axis->val();
        const float rad = _theta->val();
        const float s = Math::sin(rad / 2);
        const float c = Math::cos(rad / 2);
        return {axis.x() * s, axis.y() * s, axis.z() * s, c};
    }

private:
    sp<Numeric> _theta;
    sp<Vec3> _axis;
    V4 _val;
};

class EulerRotation : public Vec4 {
public:
    EulerRotation(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
        : _pitch(std::move(pitch)), _yaw(std::move(yaw)), _roll(std::move(roll)), _val(updateQuaternion()) {
    }

    bool update(uint64_t timestamp) override {
        if(UpdatableUtil::update(timestamp, _pitch, _yaw, _roll)) {
            _val = updateQuaternion();
            return true;
        }
        return false;
    }

    V4 val() override {
        return _val;
    }

private:
    V4 updateQuaternion() const {
        const glm::quat quat = glm::quat(glm::vec3(_pitch->val(), _yaw->val(), _roll->val()));
        return {quat.x, quat.y, quat.z, quat.w};
    }

private:
    sp<Numeric> _pitch;
    sp<Numeric> _yaw;
    sp<Numeric> _roll;
    V4 _val;
};

}


Quaternion::Quaternion(sp<Numeric> theta, sp<Vec3> axis)
    : Wrapper(sp<AxisRotation>::make(std::move(theta), std::move(axis)))
{
}

Quaternion::Quaternion(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
    : Wrapper(sp<EulerRotation>::make(std::move(pitch), std::move(yaw), std::move(roll)))
{
}

V4 Quaternion::val()
{
    return _wrapped->val();
}

bool Quaternion::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

void Quaternion::setRotation(sp<Numeric> theta, sp<Vec3> axis)
{
    _wrapped = sp<VariableDirty<V4>>::make(sp<Vec4>::make<AxisRotation>(std::move(theta), std::move(axis)), *this);
}

void Quaternion::setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
{
    _wrapped = sp<VariableDirty<V4>>::make(sp<Vec4>::make<EulerRotation>(std::move(pitch), std::move(yaw), std::move(roll)), *this);
}

}
