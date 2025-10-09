#include "graphics/inf/transform.h"

#include "core/base/constants.h"
#include "core/util/updatable_util.h"

namespace ark {

Transform::Transform()
    : _stub(sp<Stub>::make(Stub{}))
{
}

Transform::Transform(sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : _stub(sp<Stub>::make(Stub{{std::move(translation)}, {std::move(rotation), constants::QUATERNION_ONE}, {std::move(scale), constants::SCALE_ONE}}))
{
}

const OptionalVar<Vec4>& Transform::rotation() const
{
    return _stub->_rotation;
}

const OptionalVar<Vec3>& Transform::scale() const
{
    return _stub->_scale;
}

const OptionalVar<Vec3>& Transform::translation() const
{
    return _stub->_translation;
}

bool Transform::Stub::update(const uint64_t timestamp) const
{
    return UpdatableUtil::update(timestamp, _translation, _rotation, _scale, _timestamp);
}

}
