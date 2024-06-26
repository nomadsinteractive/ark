#include "core/impl/uploader/input_numeric.h"

#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

namespace ark {

InputNumeric::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _numeric(factory.ensureBuilder<Numeric>(value))
{
}

sp<Uploader> InputNumeric::BUILDER::build(const Scope& args)
{
    return sp<UploaderOfVariable<float>>::make(_numeric->build(args));
}

}
