#include "graphics/impl/vv2/vv2_add.h"

#include "graphics/base/v2.h"

namespace ark {

VV2Add::VV2Add(const sp<VV2>& lvalue, const sp<VV2>& rvalue)
    : _lvalue(lvalue), _rvalue(rvalue)
{
}

V2 VV2Add::val()
{
    return _lvalue->val() + _rvalue->val();
}

}
