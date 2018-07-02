#include "core/impl/numeric/boundary.h"

#include "core/base/bean_factory.h"
#include "core/base/expectation.h"

namespace ark {

Boundary::Boundary(const sp<Numeric>& delegate, const sp<Expectation>& expectation)
    : _delegate(delegate), _expectation(expectation), _is_greater(delegate->val() > expectation->val())
{
}

float Boundary::val()
{
    float value = _delegate->val();
    float boundary = _expectation->val();
    bool isGreater = value > boundary;
    if(isGreater != _is_greater)
    {
        _is_greater = isGreater;
        _expectation->fire();
        return boundary;
    }
    return value;
}

Boundary::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)),
      _expectation(factory.ensureBuilder<Expectation>(manifest, Constants::Attributes::EXPECTATION))
{
}

sp<Numeric> Boundary::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Boundary(_delegate->build(args), _expectation->build(args)));
}

}
