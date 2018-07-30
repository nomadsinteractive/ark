#include "core/impl/numeric/min.h"

namespace ark {

Min::Min(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
}

float Min::val()
{
    float v1 = _a1->val();
    float v2 = _a2->val();
    return v1 <= v2 ? v1 : v2;
}

sp<Numeric> Min::min(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<Numeric>::adopt(new Min(a, b));
}

}
