#include "core/impl/flatable/flatable_uint8.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

FlatableUint8::FlatableUint8(const sp<Numeric>& numeric)
    : _numeric(numeric)
{
}

void FlatableUint8::flat(void* buf)
{
    uint8_t value = static_cast<uint8_t>(_numeric->val());
    *reinterpret_cast<uint8_t*>(buf) = value;
}

uint32_t FlatableUint8::size()
{
    return 1;
}

uint32_t FlatableUint8::length()
{
    return 1;
}

FlatableUint8::BUILDER::BUILDER(BeanFactory& parent, const String &value)
    : _numeric(parent.ensureBuilder<Numeric>(value))
{
}

sp<Flatable> FlatableUint8::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableUint8>::make(_numeric->build(args));
}

}
