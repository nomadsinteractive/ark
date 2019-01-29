#include "graphics/impl/flatable/flatable_v2f.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/base/v2.h"

namespace ark {

FlatableV2f::FlatableV2f(const sp<Vec2>& vv2)
    : _vv2(vv2)
{
}

void FlatableV2f::flat(void* buf)
{
    *reinterpret_cast<V2*>(buf) = _vv2->val();
}

uint32_t FlatableV2f::size()
{
    return sizeof(V2);
}

FlatableV2f::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _vv2(parent.ensureBuilder<Vec2>(value))
{
}

sp<Flatable> FlatableV2f::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableV2f>::make(_vv2->build(args));
}



}
