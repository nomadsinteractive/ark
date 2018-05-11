#include "graphics/base/rotate.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/variable_wrapper.h"
#include "core/types/null.h"

#include "graphics/base/v3.h"

namespace ark {

const V3 Rotate::Z_AXIS = V3(0, 0, 1.0f);

Rotate::Rotate(const sp<Numeric>& rotation, const sp<VV3>& direction)
    : _rotation(sp<NumericWrapper>::make(rotation)), _direction(direction ? direction : sp<VV3::Const>::make(Z_AXIS))
{
}

sp<Numeric> Rotate::rotation() const
{
    return _rotation;
}

void Rotate::setRotation(float rotation)
{
    _rotation->set(rotation);
}

void Rotate::setRotation(const sp<Numeric>& rotation)
{
    _rotation->set(rotation);
}

const sp<VV3>& Rotate::direction() const
{
    return _direction;
}

void Rotate::setDirection(const sp<VV3>& direction)
{
    _direction = direction;
}

template<> ARK_API const sp<Rotate> Null::ptr()
{
    return Ark::instance().obtain<Rotate>(Null::ptr<Numeric>(), Ark::instance().obtain<VV3::Const>(Rotate::Z_AXIS));
}

Rotate::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _rotation(factory.getBuilder<Numeric>(manifest, "rotation"))
{
}

sp<Rotate> Rotate::BUILDER::build(const sp<Scope>& args)
{
    return sp<Rotate>::make(_rotation->build(args));
}

}
