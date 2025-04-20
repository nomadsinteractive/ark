#include "graphics/impl/uploader/uploader_vec4i.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

#include "graphics/base/v4.h"

namespace ark {

UploaderVec4i::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec4i(factory.ensureBuilder<Vec4i>(value))
{
}

sp<Uploader> UploaderVec4i::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<V4i>>(_vec4i->build(args));
}

}
