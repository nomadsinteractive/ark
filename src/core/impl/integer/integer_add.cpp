#include "core/impl/integer/integer_add.h"

namespace ark {

IntegerAdd::IntegerAdd(const sp<Integer>& lvalue, const sp<Integer>& rvalue)
    : _lvalue(lvalue), _rvalue(rvalue)
{
    DASSERT(_lvalue && _rvalue);
}

int32_t IntegerAdd::val()
{
    return _lvalue->val() + _rvalue->val();
}

}
