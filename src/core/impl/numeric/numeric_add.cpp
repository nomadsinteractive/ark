#include "core/impl/numeric/numeric_add.h"

#include "core/base/bean_factory.h"

namespace ark {

NumericAdd::NumericAdd(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
    DCHECK(a1 && a2, "Illegal argument: Null pointer");
}

float NumericAdd::val()
{
    return _a1->val() + _a2->val();
}

NumericAdd::STYLE::STYLE(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate), _plus(parent.ensureBuilder<Numeric>(value))
{
}

sp<Numeric> NumericAdd::STYLE::build(const sp<Scope>& args)
{
    return sp<NumericAdd>::adopt(new NumericAdd(_delegate->build(args), _plus->build(args)));
}

}
