#include "core/impl/numeric/fence.h"

namespace ark {

Fence::Fence(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier)
    : _delegate(delegate), _expectation(expectation), _notifer(std::move(notifier)), _is_greater(delegate->val() > expectation->val())
{
}

float Fence::val()
{
    float value = _delegate->val();
    float boundary = _expectation->val();
    bool isGreater = value > boundary;
    if(isGreater != _is_greater)
    {
        _is_greater = isGreater;
        _notifer.notify();
    }
    return value;
}

}
