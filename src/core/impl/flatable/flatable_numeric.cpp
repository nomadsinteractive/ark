#include "core/impl/flatable/flatable_numeric.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

FlatableNumeric::FlatableNumeric(const sp<Numeric>& numeric)
    : _numeric(numeric)
{
}

void FlatableNumeric::flat(void* buf)
{
    float value = _numeric->val();
    *reinterpret_cast<float*>(buf) = value;
}

uint32_t FlatableNumeric::size()
{
    return 4;
}

FlatableNumeric::BUILDER::BUILDER(BeanFactory& parent, const String &value)
    : _numeric(parent.ensureBuilder<Numeric>(value))
{
}

sp<Flatable> FlatableNumeric::BUILDER::build(const sp<Scope>& args)
{
    const sp<Numeric> numeric = _numeric->build(args);
    const sp<Changed> changed = numeric.as<Changed>();
    sp<Flatable> flatable = sp<FlatableNumeric>::make(numeric);
    if(changed)
        flatable.absorb<Changed>(changed);
    return flatable;
}

}
