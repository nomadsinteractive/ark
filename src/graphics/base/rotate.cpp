#include "graphics/base/rotate.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/null.h"
#include "core/util/holder_util.h"

#include "graphics/base/v3.h"

namespace ark {

const V3 Rotate::Z_AXIS = V3(0, 0, 1.0f);

Rotate::Rotate(const sp<Numeric>& value, const sp<Vec3>& direction)
    : _value(sp<NumericWrapper>::make(value)), _direction(direction ? direction : static_cast<sp<Vec3>>(sp<Vec3::Const>::make(Z_AXIS)))
{
}

void Rotate::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_value->delegate(), visitor);
    HolderUtil::visit(_direction, visitor);
}

float Rotate::rotation() const
{
    return _value->val();
}

void Rotate::setRotation(float rotation)
{
    _value->set(rotation);
}

void Rotate::setRotation(const sp<Numeric>& rotation)
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
    return sp<Rotate>::make(nullptr, sp<Vec3>::make<Vec3::Const>(Rotate::Z_AXIS));
}

Rotate::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _rotation(factory.getBuilder<Numeric>(manifest, Constants::Attributes::ROTATE))
{
}

sp<Rotate> Rotate::BUILDER::build(const Scope& args)
{
    return sp<Rotate>::make(_rotation->build(args));
}

Rotate::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& str)
    : _rotation(factory.getBuilder<Numeric>(str))
{
}

sp<Rotate> Rotate::DICTIONARY::build(const Scope& args)
{
    return sp<Rotate>::make(_rotation->build(args));
}

}
