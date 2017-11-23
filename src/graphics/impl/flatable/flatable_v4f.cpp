#include "graphics/impl/flatable/flatable_v4f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/color.h"
#include "graphics/base/v4.h"

namespace ark {

FlatableV4f::FlatableV4f(const sp<VV4>& vv4)
    : _vv4(vv4)
{
}

void FlatableV4f::flat(void* buf)
{
    (*reinterpret_cast<V4*>(buf)) = _vv4->val();
}

uint32_t FlatableV4f::size()
{
    return sizeof(V4);
}

uint32_t FlatableV4f::length()
{
    return 1;
}

FlatableV4f::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _vv4(parent.ensureBuilder<VV4>(value))
{
}

sp<Flatable> FlatableV4f::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableV4f>::make(_vv4->build(args));
}

}
