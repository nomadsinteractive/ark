#include "graphics/impl/uploader/uploader_vec2i.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

namespace ark {

UploaderVec2i::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec2i(factory.ensureBuilder<Vec2i>(value))
{
}

sp<Uploader> UploaderVec2i::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<V2i>>(_vec2i->build(args));
}

}
