#include "core/impl/uploader/input_int32.h"

#include "core/base/bean_factory.h"
#include "core/impl/uploader/input_variable.h"

namespace ark {

InputInt32::BUILDER::BUILDER(BeanFactory& factory, const String &value)
    : _var(factory.ensureBuilder<Integer>(value))
{
}

sp<Uploader> InputInt32::BUILDER::build(const Scope& args)
{
    return sp<InputVariable<int32_t>>::make(_var->build(args));
}

}
