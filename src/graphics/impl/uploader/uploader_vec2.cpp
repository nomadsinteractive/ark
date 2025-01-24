#include "graphics/impl/uploader/uploader_vec2.h"

#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

#include "graphics/base/v2.h"

namespace ark {

UploaderVec2::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec2(factory.ensureBuilder<Vec2>(value))
{
}

sp<Uploader> UploaderVec2::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<V2>>(_vec2->build(args));
}

}
