#include "core/impl/flatable/flatable_int32.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

FlatableInt32::FlatableInt32(const sp<Numeric>& numeric)
    : _numeric(numeric)
{
}

void FlatableInt32::flat(void* buf)
{
    int32_t value = static_cast<int32_t>(_numeric->val());
    *reinterpret_cast<int32_t*>(buf) = value;
}

uint32_t FlatableInt32::size()
{
    return 4;
}

uint32_t FlatableInt32::length()
{
    return 1;
}

FlatableInt32::BUILDER::BUILDER(BeanFactory& parent, const String &value)
    : _numeric(parent.ensureBuilder<Numeric>(value))
{
}

sp<Flatable> FlatableInt32::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableInt32>::make(_numeric->build(args));
}

}
