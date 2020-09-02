#include "graphics/base/quaternion.h"

#include <glm/gtx/quaternion.hpp>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/null.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

#include "graphics/base/v3.h"
#include "graphics/base/quaternion.h"

namespace ark {

const V3 Rotation::Z_AXIS = V3(0, 0, 1.0f);

Rotation::Rotation(float theta, const V3& axis)
{
    setRotation(theta, axis);
}

Rotation::Rotation(const sp<Numeric>& theta, const sp<Vec3>& axis)
{
    setRotation(theta, axis);
}

V4 Rotation::val()
{
    return _delegate->val();
}

bool Rotation::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

void Rotation::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_delegate, visitor);
}

const sp<Numeric>& Rotation::theta() const
{
    return _theta;
}

const sp<Vec3>& Rotation::axis() const
{
    return _axis;
}

void Rotation::setRotation(float theta, const V3& axis)
{
    setRotation(sp<Numeric::Const>::make(theta), sp<Vec3::Const>::make(axis));
}

void Rotation::setRotation(const sp<Numeric>& theta, const sp<Vec3>& axis)
{
    _theta = theta;
    _axis = axis ? axis : sp<Vec3>::make<Vec3::Const>(Rotation::Z_AXIS);

    _delegate = sp<Quaternion>::make(_theta, _axis);
}

void Rotation::setEuler(float pitch, float yaw, float roll)
{
    _delegate = sp<Quaternion>::make(sp<Numeric::Const>::make(pitch), sp<Numeric::Const>::make(yaw), sp<Numeric::Const>::make(roll));
}

void Rotation::setEuler(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll)
{
    _delegate = sp<Quaternion>::make(pitch, yaw, roll);
}

template<> ARK_API sp<Rotation> Null::ptr()
{
    return sp<Rotation>::make(sp<Numeric::Const>::make(0));
}

Rotation::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _theta(factory.getBuilder<Numeric>(manifest, Constants::Attributes::ROTATE)), _axis(factory.getBuilder<Vec3>(manifest, "axis"))
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
