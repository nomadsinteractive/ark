#include "core/base/expectation.h"

#include "core/util/bean_utils.h"

namespace ark {

Expectation::Expectation(const sp<Numeric>& expectation, const sp<Runnable>& onfire)
    : _expectation(expectation), _onfire(onfire)
{
}

float Expectation::val()
{
    return _expectation->val();
}

void Expectation::fire()
{
    _onfire.notify();
}

void Expectation::fireOnce()
{
    _onfire.notifyOnce();
}

Expectation::DICTIONARY::DICTIONARY(BeanFactory& factory, const String str)
{
    BeanUtils::parse(factory, str, _expectation, _onfire);
}

sp<Expectation> Expectation::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Expectation>::make(_expectation->build(args), _onfire->build(args));
}

}
