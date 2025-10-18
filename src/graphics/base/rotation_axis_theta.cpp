#include "graphics/base/rotation_axis_theta.h"

#include "core/util/math.h"
#include "core/util/updatable_util.h"

#include "graphics/components/rotation.h"

namespace ark {

RotationAxisTheta::RotationAxisTheta(sp<Vec3> axis, sp<Numeric> theta)
    : _axis(std::move(axis)), _theta(std::move(theta))
{
    doUpdate();
}

V4 RotationAxisTheta::val()
{
    return _val;
}

bool RotationAxisTheta::update(const uint64_t timestamp)
{
    if(UpdatableUtil::update(timestamp, _axis, _theta))
    {
        doUpdate();
        return true;
    }
    return false;
}

const sp<Vec3>& RotationAxisTheta::axis() const
{
    return _axis;
}

const sp<Numeric>& RotationAxisTheta::theta() const
{
    return _theta;
}

void RotationAxisTheta::doUpdate()
{
    const V3 axis = _axis->val();
    const float rad = _theta->val();
    const float s = Math::sin(rad / 2);
    const float c = Math::cos(rad / 2);
    _val = {axis.x() * s, axis.y() * s, axis.z() * s, c};
}

}
