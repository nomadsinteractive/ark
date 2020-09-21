#include "core/impl/flatable/flatable_numeric.h"

#include "core/base/bean_factory.h"
#include "core/impl/flatable/flatable_by_variable.h"

namespace ark {

FlatableNumeric::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _numeric(factory.ensureBuilder<Numeric>(value))
{
}

sp<Input> FlatableNumeric::BUILDER::build(const Scope& args)
{
    const sp<Numeric> numeric = _numeric->build(args);
    const sp<Notifier> notifier = numeric.as<Notifier>();
    sp<Input> flatable = sp<FlatableByVariable<float>>::make(numeric);
    if(notifier)
        flatable.absorb<Notifier>(notifier);
    return flatable;
}

}
