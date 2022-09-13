#include "core/impl/input/input_numeric.h"

#include "core/base/bean_factory.h"
#include "core/impl/input/input_variable.h"

namespace ark {

FlatableNumeric::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _numeric(factory.ensureBuilder<Numeric>(value))
{
}

sp<Input> FlatableNumeric::BUILDER::build(const Scope& args)
{
    const sp<Numeric> numeric = _numeric->build(args);
    const sp<Notifier> notifier = numeric.as<Notifier>();
    sp<Input> flatable = sp<InputVariable<float>>::make(numeric);
    if(notifier)
        flatable.absorb<Notifier>(notifier);
    return flatable;
}

}
