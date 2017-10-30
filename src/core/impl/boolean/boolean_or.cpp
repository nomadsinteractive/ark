#include "core/impl/boolean/boolean_or.h"

#include "core/base/bean_factory.h"

namespace ark {

BooleanOr::BooleanOr(const sp<Boolean>& a1, const sp<Boolean>& a2)
    : _a1(a1), _a2(a2)
{
    NOT_NULL(_a1);
    NOT_NULL(_a2);
}

bool BooleanOr::val()
{
    return _a1->val() || _a2->val();
}

}
