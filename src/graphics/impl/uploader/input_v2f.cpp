#include "graphics/impl/uploader/input_v2f.h"

#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

#include "graphics/base/v2.h"

namespace ark {

InputV2f::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec2(factory.ensureBuilder<Vec2>(value))
{
}

sp<Uploader> InputV2f::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<V2>>(_vec2->build(args));
}

}
