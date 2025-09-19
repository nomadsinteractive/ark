#include "graphics/impl/uploader/uploader_mat4.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/impl/uploader/uploader_of_variable.h"

#include "graphics/base/mat.h"

namespace ark {

UploaderMat4::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _mat4(factory.ensureBuilder<Mat4>(value))
{
}

sp<Uploader> UploaderMat4::BUILDER::build(const Scope& args)
{
    return sp<Uploader>::make<UploaderOfVariable<M4>>(_mat4->build(args));
}

}
