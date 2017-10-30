#include "core/impl/numeric/damper.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"
#include "core/impl/numeric/negative.h"

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
    : _t(factory.getBuilder<Numeric>(manifest, "t", false)), _v(factory.getBuilder<Numeric>(manifest, "v", false)),
      _s1(factory.ensureBuilder<Numeric>(manifest, "s1")), _s2(factory.ensureBuilder<Numeric>(manifest, "s2"))
{
}

sp<Numeric> Damper::BUILDER::build(const sp<Scope>& args)
{
    const sp<Numeric> t = _t ? _t->build(args) : Ark::instance().clock()->duration();
    float v = BeanUtils::toFloat(_v, args);
    float s1 = BeanUtils::toFloat(_s1, args, 0.0f);
    float s2 = BeanUtils::toFloat(_s2, args, 0.0f);
    float s0 = s1 - s2;
    float a = (v * v + s0 * s0) / s0 / 2.0f;
    float c = v2c(v, a);
    float o1 = s2 + a;
    float o2 = s2 - a;
    float asinc = a * Math::sin(c);
    float d1 = std::abs(o1 + asinc - s1);
    float d2 = std::abs(o2 + asinc - s1);
    float d3 = std::abs(o1 - asinc - s1);
    float d4 = std::abs(o2 - asinc - s1);
    const sp<Numeric> duration = c > Math::PI_2 ? t : sp<Numeric>::adopt(new Negative(t));
    if(d1 <= d2 && d1<= d3 && d1 <= d4)
        return sp<Numeric>::adopt(new Damper(duration, a, c, o1));
    if(d2 <= d1 && d2<= d3 && d2 <= d4)
        return sp<Numeric>::adopt(new Damper(duration, a, c, o2));
    if(d3 <= d1 && d3<= d2 && d3 <= d4)
        return sp<Numeric>::adopt(new Damper(duration, a, -c, o1));
    return sp<Numeric>::adopt(new Damper(duration, a, -c, o2));
}

float Damper::BUILDER::v2c(float v, float a) const
{
    float k = v / a;
    DCHECK(k >= -1.0 && k <= 1.0f, "Illegal v: %.1f a: %.1f", v, a);
    return Math::acos(k);
}

}
