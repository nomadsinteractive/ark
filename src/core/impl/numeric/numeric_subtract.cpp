#include "core/impl/numeric/numeric_subtract.h"

#include "core/base/bean_factory.h"

namespace ark {

NumericSubtract::NumericSubtract(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
    DCHECK(a1 && a2, "Illegal argument: Null pointer");
}

float NumericSubtract::val()
{
    return _a1->val() - _a2->val();
}

NumericSubtract::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Numeric> >& delegate, const String& value)
    : _delegate(delegate), _subtract(factory.ensureBuilder<Numeric>(value))
{
}

sp<Numeric> NumericSubtract::STYLE::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new NumericSubtract(_delegate->build(args), _subtract->build(args)));
}

}
