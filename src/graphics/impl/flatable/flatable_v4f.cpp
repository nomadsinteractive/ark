#include "graphics/impl/flatable/flatable_v4f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/color.h"
#include "graphics/base/v4.h"

namespace ark {

FlatableV4f::FlatableV4f(const sp<Vec4>& vec4)
    : _vec4(vec4)
{
}

void FlatableV4f::flat(void* buf)
{
    (*reinterpret_cast<V4*>(buf)) = _vec4->val();
}

uint32_t FlatableV4f::size()
{
    return sizeof(V4);
}

FlatableV4f::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _vec4(parent.ensureBuilder<Vec4>(value))
{
}

sp<Flatable> FlatableV4f::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableV4f>::make(_vec4->build(args));
}

}
