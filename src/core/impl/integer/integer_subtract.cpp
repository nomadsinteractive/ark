#include "core/impl/integer/integer_subtract.h"

namespace ark {

IntegerSubtract::IntegerSubtract(const sp<Integer>& lvalue, const sp<Integer>& rvalue)
    : _lvalue(lvalue), _rvalue(rvalue)
{
    NOT_NULL(_lvalue && _rvalue);
}

int32_t IntegerSubtract::val()
{
    return _lvalue->val() - _rvalue->val();
}

}
