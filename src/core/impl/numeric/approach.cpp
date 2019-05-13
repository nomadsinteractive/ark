#include "core/impl/numeric/approach.h"

#include "core/epi/notifier.h"
#include "core/util/math.h"

namespace ark {

Approach::Approach(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier)
    : _delegate(delegate), _expectation(expectation), _notifer(std::move(notifier)), _delegate_value(delegate->val())
{
}

float Approach::val()
{
    float value = _delegate->val();
    float expectation = _expectation->val();
    if(!Math::between(_delegate_value, expectation, value))
        _notifer.notify();
    _delegate_value = value;
    return value;
}

}
