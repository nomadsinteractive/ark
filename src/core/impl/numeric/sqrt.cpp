#include "core/impl/numeric/sqrt.h"

#include "core/util/math.h"

namespace ark {

Sqrt::Sqrt(const sp<Numeric>& x)
    : _x(x)
{
}

float Sqrt::val()
{
    return Math::sqrt(_x->val());
}

sp<Numeric> Sqrt::call(const sp<Numeric>& x)
{
    return sp<Sqrt>::make(x);
}

}
