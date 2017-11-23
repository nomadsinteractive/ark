#include "graphics/impl/flatable/flatable_v3f.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v3.h"

namespace ark {

FlatableVec3::FlatableVec3(const sp<VV3>& vv3)
    : _vv3(vv3)
{
}

void FlatableVec3::flat(void* buf)
{
    *reinterpret_cast<V3*>(buf) = _vv3->val();
}

uint32_t FlatableVec3::size()
{
    return sizeof(V3);
}

uint32_t FlatableVec3::length()
{
    return 1;
}

FlatableVec3::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _vv3(parent.ensureBuilder<VV3>(value))
{
}

sp<Flatable> FlatableVec3::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableVec3>::make(_vv3->build(args));
}

}
