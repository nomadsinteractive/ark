#include "core/impl/numeric/approach.h"

#include "core/base/notifier.h"
#include "core/util/math.h"
#include "core/util/variable_util.h"

namespace ark {

Approach::Approach(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier)
    : _delegate(delegate), _expectation(expectation), _notifer(std::move(notifier)), _delegate_value(delegate->val())
{
}

float Approach::val()
{
    return _delegate_value;
}

bool Approach::update(uint64_t timestamp)
{
    if(!VariableUtil::update(timestamp, _delegate, _expectation))
        return false;

    float value = _delegate->val();
    float expectation = _expectation->val();
    if(!Math::between(_delegate_value, expectation, value))
        _notifer.notify();
    _delegate_value = value;

    return true;
}

}
