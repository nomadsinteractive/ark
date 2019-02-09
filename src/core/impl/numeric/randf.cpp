#include "core/impl/numeric/randf.h"

#include "core/util/bean_utils.h"
#include "core/util/math.h"

namespace ark {

Randf::Randf(const sp<Numeric>& b)
    : _b(b)
{
}

Randf::Randf(const sp<Numeric>& a, const sp<Numeric>& b)
    : _a(a), _b(b)
{
}

float Randf::val()
{
    float a = _a->val();
    float b = _b->val();
    return Math::randf() * (b - a) + a;
}

sp<Numeric> Randf::randf(const sp<Numeric>& b)
{
    Randf rand(b);
    return sp<Numeric::Const>::make(rand.val());
}

sp<Numeric> Randf::randf(const sp<Numeric>& a, const sp<Numeric>& b)
{
    Randf rand(a, b);
    return sp<Numeric::Const>::make(rand.val());
}

sp<Numeric> Randf::randfv(const sp<Numeric>& b)
{
    return sp<Randf>::make(b);
}

sp<Numeric> Randf::randfv(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<Randf>::make(a, b);
}

}
