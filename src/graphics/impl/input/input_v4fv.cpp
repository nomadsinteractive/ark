#include "graphics/impl/input/input_v4fv.h"

namespace ark {

InputV4fv::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _impl(factory, value){
}

sp<Input> InputV4fv::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
