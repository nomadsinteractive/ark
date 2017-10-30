#include "core/impl/numeric/add.h"

#include "core/base/bean_factory.h"

namespace ark {

Add::Add(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
    DCHECK(a1 && a2, "Illegal argument: Null pointer");
}

float Add::val()
{
    return _a1->val() + _a2->val();
}

Add::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate), _plus(parent.ensureBuilder<Numeric>(value))
{
}

sp<Numeric> Add::DECORATOR::build(const sp<Scope>& args)
{
    return sp<Add>::adopt(new Add(_delegate->build(args), _plus->build(args)));
}

}
