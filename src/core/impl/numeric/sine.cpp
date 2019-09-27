#include "core/impl/numeric/sine.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"
#include "core/util/documents.h"

namespace ark {

Sine::Sine(const sp<Numeric>& rad)
    : _x(rad)
{
}

float Sine::val()
{
    return Math::sin(_x->val());
}

sp<Numeric> Sine::call(const sp<Numeric>& x)
{
    return sp<Sine>::make(x);
}

Sine::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.ensureBuilder<Numeric>(doc, "x"))
{
}

sp<Numeric> Sine::BUILDER::build(const Scope& args)
{
    return Sine::call(_x->build(args));
}

}
