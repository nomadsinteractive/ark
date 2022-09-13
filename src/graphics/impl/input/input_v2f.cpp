#include "graphics/impl/input/input_v2f.h"

#include "core/base/bean_factory.h"
#include "core/base/bean_factory.h"
#include "core/impl/input/input_variable.h"

#include "graphics/base/v2.h"

namespace ark {

InputV2f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec2(factory.ensureBuilder<Vec2>(value))
{
}

sp<Input> InputV2f::BUILDER::build(const Scope& args)
{
    return sp<InputVariable<V2>>::make(_vec2->build(args));
}

}
