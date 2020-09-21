#include "graphics/impl/flatable/flatable_v2f.h"

#include "core/base/bean_factory.h"
#include "core/base/bean_factory.h"
#include "core/impl/flatable/flatable_by_variable.h"

#include "graphics/base/v2.h"

namespace ark {

FlatableV2f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec2(factory.ensureBuilder<Vec2>(value))
{
}

sp<Input> FlatableV2f::BUILDER::build(const Scope& args)
{
    return sp<FlatableByVariable<V2>>::make(_vec2->build(args));
}

}
