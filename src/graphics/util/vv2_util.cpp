#include "graphics/util/vv2_util.h"

#include "graphics/base/vec2.h"
#include "graphics/impl/vv2/vv2_add.h"
#include "graphics/impl/vv2/vv2_sub.h"
#include "graphics/impl/vv2/vv2_with_transform.h"

namespace ark {

sp<VV2> VV2Util::create(const sp<Numeric>& x, const sp<Numeric>& y)
{
    return sp<Vec2>::make(x, y);
}

sp<VV2> VV2Util::create(float x, float y)
{
    return sp<Vec2>::make(x, y);
}

sp<VV2> VV2Util::add(const sp<VV2>& self, const sp<VV2>& rvalue)
{
    return sp<VV2Add>::make(self, rvalue);
}

sp<VV2> VV2Util::sub(const sp<VV2>& self, const sp<VV2>& rvalue)
{
    return sp<VV2Sub>::make(self, rvalue);
}

sp<VV2> VV2Util::mul(const sp<VV2>& self, const sp<VV2>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<VV2> VV2Util::truediv(const sp<VV2>& self, const sp<VV2>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<VV2> VV2Util::floordiv(const sp<VV2>& self, const sp<VV2>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<VV2> VV2Util::negative(const sp<VV2>& self)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<VV2> VV2Util::transform(const sp<VV2>& self, const sp<Transform>& transform, const sp<VV2>& org)
{
    return sp<VV2WithTransform>::make(self, org, transform);
}

V2 VV2Util::val(const sp<VV2>& self)
{
    return self->val();
}

}
