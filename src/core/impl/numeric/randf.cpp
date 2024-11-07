#include "core/impl/numeric/randf.h"

#include "core/util/bean_utils.h"
#include "core/util/math.h"

namespace ark {

Randf::Randf(sp<Numeric> b)
    : _b(std::move(b))
{
}

Randf::Randf(sp<Numeric> a, sp<Numeric> b)
    : _a(std::move(a)), _b(std::move(b))
{
}

float Randf::val()
{
    const float a = _a.val();
    const float b = _b->val();
    return Math::randf() * (b - a) + a;
}

bool Randf::update(uint64_t timestamp)
{
    return true;
}

sp<Numeric> Randf::randf(const sp<Numeric>& b)
{
    Randf rand(b);
    return sp<Const>::make(rand.val());
}

sp<Numeric> Randf::randf(const sp<Numeric>& a, const sp<Numeric>& b)
{
    Randf rand(a, b);
    return sp<Const>::make(rand.val());
}

sp<Numeric> Randf::randfv(const sp<Numeric>& b)
{
    return sp<Numeric>::make<Randf>(b);
}

sp<Numeric> Randf::randfv(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<Numeric>::make<Randf>(a, b);
}

}
