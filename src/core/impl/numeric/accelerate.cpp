#include "core/impl/numeric/accelerate.h"

#include "core/ark.h"
#include "core/base/duration.h"
#include "core/util/bean_utils.h"

namespace ark {

Accelerate::Accelerate(const sp<Numeric>& t, float v, float a, float s)
    : _t(t), _v(v), _a(a), _s(s)
{
}

float Accelerate::val()
{
    const float t = _t->val();
    return _s + _v * t + 0.5f * _a * t * t;
}

Accelerate::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _duration(factory.ensureBuilder<Duration>(manifest)), _v(factory.getBuilder<Numeric>(manifest, "v")),
      _a(factory.ensureBuilder<Numeric>(manifest, "a")), _s(factory.getBuilder<Numeric>(manifest, "s"))
{
}

sp<Numeric> Accelerate::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Accelerate(_duration->build(args), BeanUtils::toFloat(_v, args),
                                             BeanUtils::toFloat(_a, args), BeanUtils::toFloat(_s, args)));
}

}
