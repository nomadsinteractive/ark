#include "core/impl/flatable/flatable_int32.h"

#include "core/base/bean_factory.h"
#include "core/impl/flatable/flatable_by_variable.h"

namespace ark {

FlatableInt32::BUILDER::BUILDER(BeanFactory& factory, const String &value)
    : _var(factory.ensureBuilder<Integer>(value))
{
}

sp<Input> FlatableInt32::BUILDER::build(const Scope& args)
{
    return sp<FlatableByVariable<int32_t>>::make(_var->build(args));
}

}
