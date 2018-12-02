#include "core/impl/numeric/damper.h"

#include "core/ark.h"
#include "core/base/duration.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

namespace ark {

Damper::Damper(const sp<Numeric>& t, float a, float c, float o)
    : _t(t), _a(a), _c(c), _o(o)
{
}

float Damper::val()
{
    return _o + _a * Math::sin(_t->val() + _c);
}


Damper::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _duration(factory.ensureBuilder<Duration>(manifest)), _v(factory.getBuilder<Numeric>(manifest, "v")),
      _s1(factory.ensureBuilder<Numeric>(manifest, "s1")), _s2(factory.ensureBuilder<Numeric>(manifest, "s2"))
{
}

/*
 *      s2 >>>>>> s1 >>>>>> o <<<<<< s1 <<<<<< s2
 *
 *      o + a * sin(c) = s1         (t = 0)
 *      a * cos(c) = v              (v = f'(t))
 *      (s2 - o) ^ 2 = a ^ 2        (0 = f'(t))
 *
 *      (a * sin(c)) ^ 2 = (s1 - o) ^ 2
 *      (a * cos(c)) ^ 2 = v ^ 2
 *
 *      (s2 - o) ^ 2 = (s1 - o) ^ 2 + v ^ 2
 *      (s2 + s1 - o * 2) * (s2 - s1) = v ^ 2
 *      o = (s1 ^ 2 - s2 ^ 2 + v ^ 2) / (s1 - s2) / 2
 */

sp<Numeric> Damper::BUILDER::build(const sp<Scope>& args)
{
    const sp<Numeric> t = _duration->build(args);
    float v = BeanUtils::toFloat(_v, args);
    float s1 = BeanUtils::toFloat(_s1, args, 0.0f);
    float s2 = BeanUtils::toFloat(_s2, args, 0.0f);
    float o = (s1 * s1 - s2 * s2 + v * v) / (s1 - s2) / 2.0f;
    float a = std::abs(s2 - o);
    float c = v2c(v, a);
    return std::abs(o + a * Math::sin(c)) < std::abs(o - a * Math::sin(c)) ? sp<Numeric>::adopt(new Damper(t, a, c, o)) : sp<Numeric>::adopt(new Damper(t, a, -c, o));
}

float Damper::BUILDER::v2c(float v, float a) const
{
    float k = v / a;
    DCHECK(k >= -1.0 && k <= 1.0f, "Illegal v: %.2f a: %.2f", v, a);
    return Math::acos(k);
}

}
