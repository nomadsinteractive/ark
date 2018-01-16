#include "core/impl/integer/integer_multiply.h"

namespace ark {

IntegerMultiply::IntegerMultiply(const sp<Integer>& lvalue, const sp<Integer>& rvalue)
    : _lvalue(lvalue), _rvalue(rvalue)
{
    NOT_NULL(_lvalue && _rvalue);
}

int32_t IntegerMultiply::val()
{
    return _lvalue->val() * _rvalue->val();
}

}
