#include "core/impl/numeric/chase.h"

#include <algorithm>

#include "core/util/math.h"
#include "core/util/log.h"

namespace ark {

Chase::Chase(const sp<Numeric>& t, const sp<Numeric>& target, float s0, float eta)
    : _t(t), _target(target), _c(t->val()), _eta(eta)
{
    DCHECK(eta > 0, "ETA should greater than zero: %.2f", eta);
    doChase(s0, 0, target->val());
}

float Chase::val()
{
    float t0 = _t->val();
    float t = _t_modifier * std::min(_eta, t0 - _c) + _t0;
    float s = _a * Math::sin(t) + _o;

    float target = _target->val();
    if(target != _target_locked)
    {
        float v0 = _a * Math::cos(t);
        doChase(s, v0, target);
        _c = t0;
    }

    return s;
}

void Chase::doChase(float s0, float v0, float target)
{
    float t1;
    Math::vibrate(s0, v0, target, 0, _o, _a, _t0, t1);
    _target_locked = target;

    float eta = t1 - _t0;
    _t_modifier = eta / _eta;
}

}
