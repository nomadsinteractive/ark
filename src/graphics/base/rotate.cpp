#include "graphics/base/rotate.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/null.h"

#include "graphics/base/v3.h"

namespace ark {

const V3 Rotate::Z_AXIS = V3(0, 0, 1.0f);

Rotate::Rotate(const sp<Numeric>& value, const sp<Vec3>& direction)
    : _value(sp<NumericWrapper>::make(value)), _direction(direction ? direction : static_cast<sp<Vec3>>(sp<Vec3::Const>::make(Z_AXIS)))
{
}

float Rotate::radians() const
{
    return _value->val();
}

void Rotate::setRadians(float rotation)
{
    _value->set(rotation);
}

void Rotate::setRadians(const sp<Numeric>& rotation)
{
    _value->set(rotation);
}

const sp<Vec3>& Rotate::direction() const
{
    return _direction;
}

void Rotate::setDirection(const sp<Vec3>& direction)
{
    _direction = direction;
}

const sp<NumericWrapper>& Rotate::value() const
{
    return _value;
}

template<> ARK_API sp<Rotate> Null::ptr()
{
    return Ark::instance().obtain<Rotate>(Null::ptr<Numeric>(), Ark::instance().obtain<Vec3::Const>(Rotate::Z_AXIS));
}

Rotate::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _rotation(factory.getBuilder<Numeric>(manifest, Constants::Attributes::ROTATE))
{
}

sp<Rotate> Rotate::BUILDER::build(const sp<Scope>& args)
{
    return sp<Rotate>::make(_rotation->build(args));
}

Rotate::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
    : _rotation(factory.getBuilder<Numeric>(str))
{
}

sp<Rotate> Rotate::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Rotate>::make(_rotation->build(args));
}

}
