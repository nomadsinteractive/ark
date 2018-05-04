#include "graphics/impl/vv2/vv2_sub.h"

#include "graphics/base/v2.h"

namespace ark {

VV2Sub::VV2Sub(const sp<VV2>& lvalue, const sp<VV2>& rvalue)
    : _lvalue(lvalue), _rvalue(rvalue)
{
}

V2 VV2Sub::val()
{
    return _lvalue->val() - _rvalue->val();
}

}
