#include "core/impl/numeric/vibrate.h"

#include "core/util/math.h"

namespace ark {

Vibrate::Vibrate(const sp<Numeric>& t, float a, float c, float o)
    : _t(t), _a(a), _c(c), _o(o)
{
}

float Vibrate::val()
{
    return Math::sin(_t->val() + _c) + _o;
}

}