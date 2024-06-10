#include "graphics/traits/rotation.h"

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/impl/variable/variable_dirty.h"

#include "graphics/base/v3.h"
#include "graphics/base/quaternion.h"

namespace ark {

Rotation::Rotation(const V4& quat)
    : Rotation(sp<Vec4>::make<Vec4::Const>(quat))
{
}

Rotation::Rotation(sp<Vec4> quaternion)
    : Wrapper(sp<VariableDirty<V4>>::make(std::move(quaternion), *this))
{
}

Rotation::Rotation(float theta, const V3& axis)
    : Rotation(sp<Numeric::Const>::make(theta), sp<Vec3::Const>::make(axis))
{
}

Rotation::Rotation(sp<Numeric> theta, sp<Vec3> axis)
    : Rotation(sp<Vec4>::make<Quaternion>(theta, axis))
{
    _theta = SafeVar(std::move(theta), 0);
    _axis = SafeVar(std::move(axis), constants::AXIS_Z);
}

V4 Rotation::val()
{
    return _wrapped->val();
}

bool Rotation::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

const SafeVar<Numeric>& Rotation::theta() const
{
    return _theta;
}

void Rotation::setTheta(sp<Numeric> theta)
{
    CHECK(_theta, "Theta can only be set in Theta-Axis mode");
    _theta.reset(std::move(theta));
    setRotation(std::move(theta), _axis.wrapped());
}

const SafeVar<Vec3>& Rotation::axis() const
{
    return _axis;
}

void Rotation::setRotation(float theta, const V3& axis)
{
    setRotation(sp<Numeric::Const>::make(theta), sp<Vec3::Const>::make(axis));
}

void Rotation::setRotation(sp<Numeric> theta, sp<Vec3> axis)
{
    _theta.reset(theta);
    _axis.reset(axis);

    _wrapped = sp<VariableDirty<V4>>::make(sp<Quaternion>::make(std::move(theta), std::move(axis)), *this);
}

void Rotation::setEuler(float pitch, float yaw, float roll)
{
    setEuler(sp<Numeric::Const>::make(pitch), sp<Numeric::Const>::make(yaw), sp<Numeric::Const>::make(roll));
}

void Rotation::setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll)
{
    _theta.reset(nullptr);
    _axis.reset(nullptr);

    _wrapped = sp<VariableDirty<V4>>::make(sp<Quaternion>::make(std::move(pitch), std::move(yaw), std::move(roll)), *this);
}

Rotation::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _theta(factory.getBuilder<Numeric>(manifest, "theta")), _axis(factory.getBuilder<Vec3>(manifest, "axis"))
{
}

sp<Rotation> Rotation::BUILDER::build(const Scope& args)
{
    return sp<Rotation>::make(_theta->build(args), _axis->build(args));
}

Rotation::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
    : _theta(factory.getBuilder<Numeric>(str))
{
}

sp<Rotation> Rotation::DICTIONARY::build(const Scope& args)
{
    return sp<Rotation>::make(_theta->build(args));
}

}
