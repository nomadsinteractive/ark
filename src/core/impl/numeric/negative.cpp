#include "core/impl/numeric/negative.h"

namespace ark {

Negative::Negative(const sp<Numeric>& a1)
    : _a1(a1)
{
}

float Negative::val()
{
    return 0.0f - _a1->val();
}

}
