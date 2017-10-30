#include "core/impl/numeric/subtract.h"

#include "core/base/bean_factory.h"

namespace ark {

Subtract::Subtract(const sp<Numeric>& a1, const sp<Numeric>& a2)
    : _a1(a1), _a2(a2)
{
    DCHECK(a1 && a2, "Illegal argument: Null pointer");
}

float Subtract::val()
{
    return _a1->val() - _a2->val();
}

Subtract::DECORATOR::DECORATOR(BeanFactory& factory, const sp<Builder<Numeric> >& delegate, const String& value)
    : _delegate(delegate), _subtract(factory.ensureBuilder<Numeric>(value))
{
}

sp<Numeric> Subtract::DECORATOR::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Subtract(_delegate->build(args), _subtract->build(args)));
}

}
