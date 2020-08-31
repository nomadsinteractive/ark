#include "graphics/base/quaternion.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/null.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

#include "graphics/base/v3.h"

namespace ark {

const V3 Quaternion::Z_AXIS = V3(0, 0, 1.0f);

Quaternion::Quaternion(const sp<Numeric>& value, const sp<Vec3>& direction)
    : _value(sp<NumericWrapper>::make(value)), _direction(direction ? direction : static_cast<sp<Vec3>>(sp<Vec3::Const>::make(Z_AXIS)))
{
}

Rotation Quaternion::val()
{
    return Rotation(_value->val(), _direction->val());
}

bool Quaternion::update(uint64_t timestamp)
{
    return VariableUtil::update(timestamp, _value, _direction);
}

void Quaternion::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_value->delegate(), visitor);
    HolderUtil::visit(_direction, visitor);
}

float Quaternion::rotation() const
{
    return _value->val();
}

void Quaternion::setRotation(float rotation)
{
    _value->set(rotation);
}

void Quaternion::setRotation(const sp<Numeric>& rotation)
{
    _value->set(rotation);
}

const sp<Vec3>& Quaternion::direction() const
{
    return _direction;
}

void Quaternion::setDirection(const sp<Vec3>& direction)
{
    _direction = direction;
}

const sp<NumericWrapper>& Quaternion::value() const
{
    return _value;
}

template<> ARK_API sp<Quaternion> Null::ptr()
{
    return sp<Quaternion>::make(nullptr, sp<Vec3>::make<Vec3::Const>(Quaternion::Z_AXIS));
}

Quaternion::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _angle(factory.getBuilder<Numeric>(manifest, Constants::Attributes::ROTATE)), _direction(factory.getBuilder<Vec3>(manifest, "direction"))
{
}

sp<Quaternion> Quaternion::BUILDER::build(const Scope& args)
{
    return sp<Quaternion>::make(_angle->build(args), _direction->build(args));
}

Quaternion::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
    : _rotation(factory.getBuilder<Numeric>(str))
{
}

sp<Quaternion> Quaternion::DICTIONARY::build(const Scope& args)
{
    return sp<Quaternion>::make(_rotation->build(args));
}

Rotation::Rotation()
    : angle(0), direction(Quaternion::Z_AXIS)
{
}

Rotation::Rotation(float angle, const V3& direction)
    : angle(angle), direction(direction)
{
}

}
