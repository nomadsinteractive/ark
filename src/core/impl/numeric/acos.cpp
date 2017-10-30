#include "core/impl/numeric/acos.h"

#include "core/util/math.h"
#include "core/base/bean_factory.h"

namespace ark {

Acos::Acos(const sp<Numeric>& x)
    : _x(x)
{
}

float Acos::val()
{
    return Math::acos(_x->val());
}

sp<Numeric> Acos::call(const sp<Numeric>& x)
{
    return sp<Acos>::make(x);
}

Acos::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.ensureBuilder<Numeric>(doc, "x"))
{
}

sp<Numeric> Acos::BUILDER::build(const sp<Scope>& args)
{
    return sp<Acos>::make(_x->build(args));
}

}
