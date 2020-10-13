#include "graphics/impl/input/flatable_v4f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/flatable/flatable_by_variable.h"

#include "graphics/base/v4.h"

namespace ark {

FlatableV4f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec4(factory.ensureBuilder<Vec4>(value))
{
}

sp<Input> FlatableV4f::BUILDER::build(const Scope& args)
{
    return sp<FlatableByVariable<V4>>::make(_vec4->build(args));
}

}
