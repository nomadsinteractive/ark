#include "graphics/base/quaternion.h"

#include <glm/gtx/quaternion.hpp>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/null.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"
#include "core/util/math.h"

#include "graphics/base/v3.h"
#include "graphics/util/vec3_type.h"

namespace ark {

namespace {

class AxisRotation : public Vec4 {
public:
    AxisRotation(const sp<Numeric>& theta, const sp<Vec3>& axis)
        : _theta(theta), _axis(Vec3Type::normalize(axis)), _val(updateQuaternion()) {
    }

    virtual bool update(uint64_t timestamp) override {
        if(VariableUtil::update(timestamp, _theta, _axis)) {
            _val = updateQuaternion();
            return true;
        }
        return false;
    }

    virtual V4 val() override {
        return _val;
    }

private:
    V4 updateQuaternion() const {
        const V3 axis = _axis->val();
        const float rad = _theta->val();
        const float s = Math::sin(rad / 2);
        const float c = Math::cos(rad / 2);
        return V4(axis.x() * s, axis.y() * s, axis.z() * s, c);
    }

private:
    sp<Numeric> _theta;
    sp<Vec3> _axis;

    V4 _val;
};

class EulerRotation : public Vec4 {
public:
    EulerRotation(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll)
        : _pitch(pitch), _yaw(yaw), _roll(roll), _val(updateQuaternion()) {
    }

    virtual bool update(uint64_t timestamp) override {
        if(VariableUtil::update(timestamp, _pitch, _yaw, _roll)) {
            _val = updateQuaternion();
            return true;
        }
        return false;
    }

    virtual V4 val() override {
        return _val;
    }

private:
    V4 updateQuaternion() const {
        const glm::quat quat = glm::quat(glm::vec3(_pitch->val(), _yaw->val(), _roll->val()));
        return V4(quat.x, quat.y, quat.z, quat.w);
    }

private:
    sp<Numeric> _pitch;
    sp<Numeric> _yaw;
    sp<Numeric> _roll;

    V4 _val;
};

}


Quaternion::Quaternion(const sp<Numeric>& rad, const sp<Vec3>& axis)
    : _delegate(sp<AxisRotation>::make(rad, axis))
{
}

Quaternion::Quaternion(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll)
    : _delegate(sp<EulerRotation>::make(pitch, yaw, roll))
{
}

V4 Quaternion::val()
{
    return _delegate->val();
}

bool Quaternion::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void Quaternion::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_delegate, visitor);
}

void Quaternion::setRotation(const sp<Numeric>& rad, const sp<Vec3>& axis)
{
    _delegate = sp<AxisRotation>::make(rad, axis);
}

void Quaternion::setEuler(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll)
{
    _delegate = sp<EulerRotation>::make(pitch, yaw, roll);
}

}
