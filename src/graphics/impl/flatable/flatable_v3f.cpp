#include "graphics/impl/flatable/flatable_v3f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/flatable/flatable_by_variable.h"

#include "graphics/base/v3.h"

namespace ark {

FlatableV3f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec3(factory.ensureBuilder<Vec3>(value))
{
}

sp<Flatable> FlatableV3f::BUILDER::build(const Scope& args)
{
    return sp<FlatableByVariable<V3>>::make(_vec3->build(args));
}

}
