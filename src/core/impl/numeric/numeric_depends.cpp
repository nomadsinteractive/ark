#include "core/impl/numeric/numeric_depends.h"

#include "core/base/bean_factory.h"

namespace ark {

NumericDepends::NumericDepends(const sp<Numeric>& delegate, const sp<Numeric>& depends)
    : _delegate(delegate), _depends(depends), _val_delegate(delegate->val()), _val_depends(depends->val())
{
}

float NumericDepends::val()
{
    float valDepends = _depends->val();
    if(valDepends != _val_depends)
    {
        _val_depends = valDepends;
        _val_delegate = _delegate->val();
    }
    return _val_delegate;
}

NumericDepends::DECORATOR::DECORATOR(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& style)
    : _delegate(delegate), _depends(beanFactory.ensureBuilder<Numeric>(style))
{
}

sp<Numeric> NumericDepends::DECORATOR::build(const sp<Scope>& args)
{
    return sp<NumericDepends>::make(_delegate->build(args), _depends->build(args));
}

}
