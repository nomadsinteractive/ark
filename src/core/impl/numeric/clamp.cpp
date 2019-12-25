#include "core/impl/numeric/clamp.h"

#include "core/util/variable_util.h"


namespace ark {

Clamp::Clamp(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, Notifier notifier)
    : Updatable(delegate->val()), _delegate(delegate), _min(min), _max(max), _notifier(std::move(notifier))
{
}

bool Clamp::doUpdate(uint64_t timestamp, float& value)
{
    if(!VariableUtil::update(timestamp, _delegate, _min, _max))
        return false;

    value = _delegate->val();
    float t;
    if((t = _min->val()) > value)
    {
        _notifier.notify();
        value = t;
    }
    else if((t = _max->val()) < value)
    {
        _notifier.notify();
        value = t;
    }

    return true;
}

}
