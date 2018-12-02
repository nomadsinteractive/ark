#include "core/impl/numeric/integral.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/duration.h"
#include "core/util/bean_utils.h"

namespace ark {

Integral::Integral(float s, const sp<Numeric>& t, const sp<Numeric>& velocity)
    : _s(s), _last_v(0), _last_t(0), _t(t), _v(velocity)
{
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

Integral::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _duration(factory.ensureBuilder<Duration>(manifest)), _s(factory.getBuilder<Numeric>(manifest, "s")), _v(factory.ensureBuilder<Numeric>(manifest, "v"))
{
}

sp<Numeric> Integral::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Integral(BeanUtils::toFloat(_s, args), _duration->build(args), _v->build(args)));
}

}
