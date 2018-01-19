#include "core/impl/numeric/numeric_multiply.h"

#include "core/base/bean_factory.h"

namespace ark {

NumericMultiply::NumericMultiply(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
    NOT_NULL(a1 && a2);
}

float NumericMultiply::val()
{
    return _a1->val() * _a2->val();
}

NumericMultiply::STYLE::STYLE(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& style)
    : _delegate(delegate), _multiply(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Numeric> NumericMultiply::STYLE::build(const sp<Scope>& args)
{
    return sp<NumericMultiply>::make(_delegate->build(args), _multiply->build(args));
}

}
