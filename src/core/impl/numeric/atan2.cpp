#include "core/impl/numeric/atan2.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

namespace ark {

Atan2::Atan2(const sp<Numeric>& y, const sp<Numeric>& x)
    : _x(x), _y(y)
{
}

float Atan2::val()
{
    return Math::atan2(_y->val(), _x->val());
}

sp<Numeric> Atan2::call(const sp<Numeric>& y, const sp<Numeric>& x)
{
    return sp<Atan2>::make(y, x);
}

Atan2::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _y(parent.ensureBuilder<Numeric>(doc, "y")), _x(parent.ensureBuilder<Numeric>(doc, "x"))
{
}

sp<Numeric> Atan2::BUILDER::build(const sp<Scope>& args)
{
    return sp<Atan2>::make(_y->build(args), _x->build(args));
}

}
