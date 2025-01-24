#include "core/impl/uploader/uploader_int32.h"

#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

namespace ark {

UploaderInt32::BUILDER::BUILDER(BeanFactory& factory, const String &value)
    : _var(factory.ensureBuilder<Integer>(value))
{
}

sp<Uploader> UploaderInt32::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<int32_t>>(_var->build(args));
}

}
