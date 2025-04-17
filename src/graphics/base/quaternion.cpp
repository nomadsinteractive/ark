#include "graphics/base/quaternion.h"

#include <glm/gtx/quaternion.hpp>

#include "core/ark.h"
#include "core/base/constants.h"
#include "core/impl/variable/variable_dirty_mark.h"
#include "core/util/updatable_util.h"
#include "core/util/math.h"

#include "graphics/base/v3.h"
#include "graphics/util/vec3_type.h"

namespace ark {

namespace {

class AxisRotation final : public Vec4 {
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

struct MatrixQuaternion final : Mat4 {
    MatrixQuaternion(sp<Vec4> quaternion)
        : _quaternion(std::move(quaternion)) {
    }

    bool update(uint64_t timestamp) override
    {
        return _quaternion->update(timestamp);
    }

    M4 val() override
    {
        const V4 quaternion = _quaternion->val();
        return {glm::toMat4(glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z()))};
    }

    sp<Vec4> _quaternion;
};

}

Quaternion::Quaternion(sp<Vec4> quaternion)
    : Wrapper(std::move(quaternion))
{
}

Quaternion::Quaternion(sp<Numeric> theta, sp<Vec3> axis)
    : Wrapper(sp<Vec4>::make<AxisRotation>(std::move(theta), std::move(axis)))
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
    _wrapped = sp<VariableDirtyMark<V4>>::make(sp<Vec4>::make<AxisRotation>(std::move(theta), std::move(axis)), *this);
}

sp<Mat4> Quaternion::toMatrix() const
{
    return sp<Mat4>::make<MatrixQuaternion>(_wrapped);
}

}
