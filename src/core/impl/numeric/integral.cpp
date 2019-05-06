#include "core/impl/numeric/integral.h"

namespace ark {

Integral::Integral(const sp<Numeric>& v, const sp<Numeric>& t)
    : _v(v), _t(t), _s(0)
{
    DASSERT(_v && _t);
    _last_v = _v->val();
    _last_t = _t->val();
}

float Integral::val()
{
    float v = _v->val();
    float t = _t->val();
    _s += (t - _last_t) * ((v + _last_v) / 2);
    _last_v = v;
    _last_t = t;
    return _s;
}

}
