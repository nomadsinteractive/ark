#include "graphics/inf/transform.h"

#include "core/base/constants.h"
#include "core/util/updatable_util.h"

#include "graphics/components/rotation.h"
#include "graphics/components/scale.h"

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
    return _stub->_pivot;
}

void Transform::onPoll(WiringContext& context)
{
    if(!context.hasInterface<Rotation>())
        context.setInterface(sp<Rotation>::make(_stub->_rotation.toVar(), _stub->_rotation.toWrapper()));
    if(!context.hasInterface<Scale>())
        context.setInterface(sp<Scale>::make(_stub->_scale.toVar(), _stub->_scale.toWrapper()));
}

void Transform::onWire(const WiringContext& context, const Box& self)
{
    if(sp<Vec4> rotation = context.getComponent<Rotation>())
        _stub->_rotation.reset(std::move(rotation));
    if(sp<Vec3> scale = context.getComponent<Scale>())
        _stub->_scale.reset(std::move(scale));
}

bool Transform::Stub::update(const uint32_t timestamp) const
{
    return UpdatableUtil::update(timestamp, _pivot, _rotation, _scale, _timestamp);
}

}
