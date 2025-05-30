#include "core/impl/uploader/uploader_uint8.h"

#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

namespace ark {

UploaderUint8::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _var(factory.ensureBuilder<Integer>(value))
{
}

sp<Uploader> UploaderUint8::BUILDER::build(const Scope& args)
{
    return sp<UploaderOfVariable<uint8_t, int32_t>>::make(_var->build(args));
}

}
