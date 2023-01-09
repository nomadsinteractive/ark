#include "graphics/base/rotation.h"

#include <glm/gtx/quaternion.hpp>

#include "core/base/bean_factory.h"
#include "core/types/null.h"
#include "core/util/holder_util.h"

#include "graphics/base/v3.h"
#include "graphics/base/quaternion.h"

namespace ark {

const V3 Rotation::Z_AXIS = V3(0, 0, 1.0f);

Rotation::Rotation(const V4& quat)
    : Rotation(nullptr, nullptr, sp<Vec4::Const>::make(quat))
{
}

Rotation::Rotation(float theta, const V3& axis)
    : Rotation(sp<Numeric::Const>::make(theta), sp<Vec3::Const>::make(axis), nullptr)
{
}

Rotation::Rotation(const sp<Numeric>& theta, const sp<Vec3>& axis)
    : Rotation(theta, axis, nullptr)
{
}

Rotation::Rotation(sp<Numeric> theta, sp<Vec3> axis, sp<Vec4> quaternion)
    : _theta(std::move(theta), 0), _axis(std::move(axis), Z_AXIS), _quaternion(quaternion ? std::move(quaternion) : sp<Vec4>::make<Quaternion>(_theta.ensure(), _axis.ensure()))
{
//TODO: _quaternion to SafeVar
    _timestamp.setDirty();
}

V4 Rotation::val()
{
    return _quaternion->val();
}

bool Rotation::update(uint64_t timestamp)
{
    return _quaternion->update(timestamp) || _timestamp.update(timestamp);
}

void Rotation::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_quaternion, visitor);
}

const sp<Numeric>& Rotation::theta()
{
    return _theta.ensure();
}

void Rotation::setTheta(const sp<Numeric>& theta)
{
    CHECK(_theta, "Theta can only be set in Theta-Axis mode");
    _theta.reset(theta);

    _quaternion = sp<Quaternion>::make(_theta.ensure(), _axis.ensure());
    _timestamp.setDirty();
}

const sp<Vec3>& Rotation::axis()
{
    return _axis.ensure();
}

void Rotation::setRotation(float theta, const V3& axis)
{
    setRotation(sp<Numeric::Const>::make(theta), sp<Vec3::Const>::make(axis));
}

void Rotation::setRotation(const sp<Numeric>& theta, const sp<Vec3>& axis)
{
    _theta.reset(theta);
    _axis.reset(axis);

    _quaternion = sp<Quaternion>::make(_theta.ensure(), _axis.ensure());
    _timestamp.setDirty();
}

void Rotation::setEuler(float pitch, float yaw, float roll)
{
    setEuler(sp<Numeric::Const>::make(pitch), sp<Numeric::Const>::make(yaw), sp<Numeric::Const>::make(roll));
}

void Rotation::setEuler(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll)
{
    _theta.reset(nullptr);
    _axis.reset(nullptr);

    _quaternion = sp<Quaternion>::make(pitch, yaw, roll);
    _timestamp.setDirty();
}

template<> ARK_API sp<Rotation> Null::safePtr()
{
    return sp<Rotation>::make(sp<Numeric>::make<Numeric::Const>(0.0f));
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
