#include "core/impl/numeric/upper.h"

#include "core/base/bean_factory.h"
#include "core/base/expectation.h"

namespace ark {

Upper::Upper(const sp<Numeric>& delegate, const sp<Expectation>& expectation)
    : _delegate(delegate), _expectation(expectation)
{
}

float Upper::val()
{
    float value = _delegate->val();
    if(value > _expectation->val())
        _expectation->fireOnce();
    return value;
}

Upper::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)),
      _expectation(factory.ensureBuilder<Expectation>(manifest, Constants::Attributes::EXPECT))
{
}

sp<Numeric> Upper::BUILDER::build(const sp<Scope>& args)
{
    return sp<Upper>::make(_delegate->build(args), _expectation->build(args));
}

}
