#include "graphics/base/rotation.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/null.h"

#include "graphics/base/v3.h"

namespace ark {

const V3 Rotation::Z_AXIS = V3(0, 0, 1.0f);

Rotation::Rotation(const sp<Numeric>& value, const sp<Vec3>& direction)
    : _value(sp<NumericWrapper>::make(value)), _direction(direction ? direction : static_cast<sp<Vec3>>(sp<Vec3::Const>::make(Z_AXIS)))
{
}

float Rotation::radians() const
{
    return _value->val();
}

void Rotation::setRadians(float rotation)
{
    _value->set(rotation);
}

void Rotation::setRadians(const sp<Numeric>& rotation)
{
    _value->set(rotation);
}

const sp<Vec3>& Rotation::direction() const
{
    return _direction;
}

void Rotation::setDirection(const sp<Vec3>& direction)
{
    _direction = direction;
}

const sp<NumericWrapper>& Rotation::value() const
{
    return _value;
}

template<> ARK_API sp<Rotation> Null::ptr()
{
    return Ark::instance().obtain<Rotation>(Null::ptr<Numeric>(), Ark::instance().obtain<Vec3::Const>(Rotation::Z_AXIS));
}

Rotation::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _rotation(factory.getBuilder<Numeric>(manifest, Constants::Attributes::ROTATION))
{
}

sp<Rotation> Rotation::BUILDER::build(const sp<Scope>& args)
{
    return sp<Rotation>::make(_rotation->build(args));
}

Rotation::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
    : _rotation(factory.getBuilder<Numeric>(str))
{
}

sp<Rotation> Rotation::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Rotation>::make(_rotation->build(args));
}

}
