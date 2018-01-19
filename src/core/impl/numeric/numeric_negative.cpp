#include "core/impl/numeric/numeric_negative.h"

namespace ark {

NumericNegative::NumericNegative(const sp<Numeric>& a1)
    : _a1(a1)
{
}

float NumericNegative::val()
{
    return 0.0f - _a1->val();
}

}
