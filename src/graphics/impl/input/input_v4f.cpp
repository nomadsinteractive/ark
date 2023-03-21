#include "graphics/impl/input/input_v4f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/input_variable.h"

#include "graphics/base/v4.h"

namespace ark {

InputV4f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec4(factory.ensureBuilder<Vec4>(value))
{
}

sp<Uploader> InputV4f::BUILDER::build(const Scope& args)
{
    return sp<InputVariable<V4>>::make(_vec4->build(args));
}

}
