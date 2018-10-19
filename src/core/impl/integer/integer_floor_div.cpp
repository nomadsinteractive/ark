#include "core/impl/integer/integer_floor_div.h"

namespace ark {

IntegerFloorDiv::IntegerFloorDiv(const sp<Integer>& lvalue, const sp<Integer>& rvalue)
    : _lvalue(lvalue), _rvalue(rvalue)
{
    DASSERT(_lvalue && _rvalue);
}

int32_t IntegerFloorDiv::val()
{
    int32_t rvalue = _rvalue->val();
    DCHECK(rvalue, "Division by zero");
    return _lvalue->val() / _rvalue->val();
}

}
