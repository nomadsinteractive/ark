#include "graphics/base/euler.h"

#include "core/util/updatable_util.h"

#include <glm/gtx/quaternion.hpp>

namespace ark {

Euler::Euler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
    : _pitch(std::move(pitch)), _yaw(std::move(yaw)), _roll(std::move(roll))
{
    doUpdate();
}

bool Euler::update(const uint64_t timestamp)
{
    if(UpdatableUtil::update(timestamp, _pitch, _yaw, _roll))
    {
        doUpdate();
        return true;
    }
    return false;
}

void Euler::doUpdate()
{
    const glm::quat quat(glm::vec3(_pitch->val(), _yaw->val(), _roll->val()));
    _val = {quat.x, quat.y, quat.z, quat.w};
}

V4 Euler::val()
{
    return _val;
}

}
