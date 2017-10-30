#include "core/impl/numeric/abs.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"
#include "core/util/documents.h"
#include "core/util/bean_utils.h"

namespace ark {

Abs::Abs(const sp<Numeric>& x)
    : _x(x)
{
}

float Abs::val()
{
    return Math::abs(_x->val());
}

sp<Numeric> Abs::call(const sp<Numeric>& x)
{
    return sp<Abs>::make(x);
}

Abs::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.ensureBuilder<Numeric>(doc, "x"))
{
}

sp<Numeric> Abs::BUILDER::build(const sp<Scope>& args)
{
    return Abs::call(_x->build(args));
}

}
