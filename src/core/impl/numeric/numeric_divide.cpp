#include "core/impl/numeric/numeric_divide.h"

#include "core/base/bean_factory.h"

namespace ark {

NumericDivide::NumericDivide(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
    DCHECK(a1 && a2, "Illegal argument: Null pointer");
}

float NumericDivide::val()
{
    return _a1->val() / _a2->val();
}

NumericDivide::DECORATOR::DECORATOR(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& style)
    : _delegate(delegate), _divide(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Numeric> NumericDivide::DECORATOR::build(const sp<Scope>& args)
{
    return sp<NumericDivide>::make(_delegate->build(args), _divide->build(args));
}

}
