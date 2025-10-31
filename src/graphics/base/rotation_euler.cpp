#include "graphics/base/rotation_euler.h"

#include <glm/gtx/quaternion.hpp>

#include "core/util/updatable_util.h"

#include "graphics/components/rotation.h"

namespace ark {

RotationEuler::RotationEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
    : _pitch(std::move(pitch)), _yaw(std::move(yaw)), _roll(std::move(roll))
{
    doUpdate();
}

V4 RotationEuler::val()
{
    return _val;
}

bool RotationEuler::update(uint32_t tick)
{
    if(UpdatableUtil::update(tick, _pitch, _yaw, _roll))
    {
        doUpdate();
        return true;
    }
    return false;
}

const sp<Numeric>& RotationEuler::pitch() const
{
    return _pitch;
}

const sp<Numeric>& RotationEuler::yaw() const
{
    return _yaw;
}

const sp<Numeric>& RotationEuler::roll() const
{
    return _roll;
}

void RotationEuler::doUpdate()
{
    const glm::quat quat(glm::vec3(_pitch->val(), _yaw->val(), _roll->val()));
    _val = {quat.x, quat.y, quat.z, quat.w};
}

}
