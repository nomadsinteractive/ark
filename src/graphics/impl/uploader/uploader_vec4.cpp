#include "graphics/impl/uploader/uploader_vec4.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

#include "graphics/base/v4.h"

namespace ark {

UploaderVec4::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _vec4(factory.ensureBuilder<Vec4>(value))
{
}

sp<Uploader> UploaderVec4::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<V4>>(_vec4->build(args));
}

}
