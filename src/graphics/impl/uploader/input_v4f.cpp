#include "graphics/impl/uploader/input_v4f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

#include "graphics/base/v4.h"

namespace ark {

InputV4f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec4(factory.ensureBuilder<Vec4>(value))
{
}

sp<Uploader> InputV4f::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<V4>>(_vec4->build(args));
}

}
