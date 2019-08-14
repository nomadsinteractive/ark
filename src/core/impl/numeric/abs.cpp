#include "core/impl/numeric/abs.h"

#include "core/util/math.h"

namespace ark {

Abs::Abs(const sp<Numeric>& x)
    : _x(x)
{
}

float Abs::val()
{
    return Math::abs(_x->val());
}

sp<Numeric> Abs::call(const sp<Numeric>& x)
{
    return sp<Abs>::make(x);
}

}
