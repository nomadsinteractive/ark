#include "core/impl/numeric/fence.h"

#include "core/util/variable_util.h"

namespace ark {

Fence::Fence(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier)
    : Updatable(delegate->val()), _delegate(delegate), _expectation(expectation), _notifer(std::move(notifier)), _is_greater(val() > expectation->val())
{
}

bool Fence::doUpdate(uint64_t timestamp, float& value)
{
    if(!VariableUtil::update(timestamp, _delegate, _expectation))
        return false;

    value = _delegate->val();
    float boundary = _expectation->val();
    bool isGreater = value > boundary;
    if(isGreater != _is_greater)
    {
        _is_greater = isGreater;
        _notifer.notify();
    }
    return true;
}

}
