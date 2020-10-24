#include "core/impl/numeric/stalker.h"

#include <algorithm>

#include "core/util/math.h"
#include "core/util/log.h"

namespace ark {

Stalker::Stalker(const sp<Numeric>& t, const sp<Numeric>& target, float s0, float eta)
    : _t(t), _target(target), _c(_t->val()), _c0(_c), _eta(eta), _s(s0)
{
    DCHECK(eta > 0, "ETA should greater than zero: %.2f", eta);
    doChase(0, _target->val(), 0);
}

float Stalker::val()
{
    return _s;
}

bool Stalker::update(uint64_t timestamp)
{
    bool targetDirty = _target->update(timestamp);
    if(_t->update(timestamp))
    {
        float t0 = _t->val();
        doUpdate(t0, targetDirty);
        return true;
    }
    return false;
}

void Stalker::doUpdate(float t0, bool targetDirty)
{
    if((t0 - _c0) >= _eta)
        _s = _target->val();
    else
    {
        float t = _t_modifier * std::min(_eta, t0 - _c) + _t0;
        _s = _a * Math::sin(t) + _o;

        float target = _target->val();
        if(targetDirty)
        {
            float v0 = _a * Math::cos(t);
            doChase(v0, target, t0 - _c0);
            _c = t0;
        }
    }
}

void Stalker::doChase(float v0, float target, float dt)
{
    float t1;
    Math::vibrate(_s, v0, target, 0, _o, _a, _t0, t1);

    float eta = t1 - _t0;
    _t_modifier = eta / (_eta - dt);
}

}
