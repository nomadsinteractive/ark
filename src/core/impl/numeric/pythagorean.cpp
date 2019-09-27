#include "core/impl/numeric/pythagorean.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"

namespace ark {

Pythagorean::Pythagorean(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
}

float Pythagorean::val()
{
    float c = _a1->val();
    float a = _a2->val();
    DCHECK(c >= a, "Illegal Pythagoras arguments, c: %.1f, a:%.1f", c, a);
    return Math::sqrt(c * c - a * a);
}

Pythagorean::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _a1(beanFactory.ensureBuilder<Numeric>(manifest, "a1")), _a2(beanFactory.ensureBuilder<Numeric>(manifest, "a2"))
{
}

sp<Numeric> Pythagorean::BUILDER::build(const Scope& args)
{
    return sp<Pythagorean>::make(_a1->build(args), _a2->build(args));
}

}
