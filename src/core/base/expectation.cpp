#include "core/base/expectation.h"

#include "core/util/numeric_util.h"
#include "core/util/bean_utils.h"

namespace ark {

Expectation::Expectation(const sp<Numeric>& expectation, const sp<Runnable>& onfire, bool fireOnce)
    : _expectation(expectation), _observer(onfire, fireOnce)
{
}

float Expectation::val()
{
    return _expectation->val();
}

void Expectation::setVal(float val)
{
    NumericUtil::setVal(_expectation, val);
}

void Expectation::fire()
{
    _observer.update();
}

Expectation::DICTIONARY::DICTIONARY(BeanFactory& factory, const String str)
{
    BeanUtils::split(factory, str, _expectation, _onfire);
}

sp<Expectation> Expectation::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Expectation>::make(_expectation->build(args), _onfire->build(args));
}

}
