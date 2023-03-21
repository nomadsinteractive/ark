#include "graphics/impl/input/input_v3f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/input_variable.h"

#include "graphics/base/v3.h"

namespace ark {

FlatableV3f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec3(factory.ensureBuilder<Vec3>(value))
{
}

sp<Uploader> FlatableV3f::BUILDER::build(const Scope& args)
{
    return sp<InputVariable<V3>>::make(_vec3->build(args));
}

}
