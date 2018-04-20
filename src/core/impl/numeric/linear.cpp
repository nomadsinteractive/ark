#include "core/impl/numeric/linear.h"

#include "core/ark.h"
#include "core/base/duration.h"
#include "core/util/bean_utils.h"

namespace ark {

Linear::Linear(const sp<Numeric>& t, float v, float s)
    : _t(t), _v(v), _s(s)
{
}

float Linear::val()
{
    return _s + _v * _t->val();
}

Linear::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _duration(factory.ensureBuilder<Duration>(manifest)), _v(factory.ensureBuilder<Numeric>(manifest, "v")), _s(factory.getBuilder<Numeric>(manifest, "s"))
{
}

sp<Numeric> Linear::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Linear(_duration->build(args), _v->build(args)->val(), BeanUtils::toFloat(_s, args)));
}

}
