#include "core/impl/numeric/clamp.h"

namespace ark {

Clamp::Clamp(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, Notifier notifier)
    : _delegate(delegate), _min(min), _max(max), _notifier(std::move(notifier))
{
}

float Clamp::val()
{
    float v = _delegate->val();
    float t;
    if((t = _min->val()) > v)
    {
        _notifier.notify();
        return t;
    }
    if((t = _max->val()) < v)
    {
        _notifier.notify();
        return t;
    }
    return v;
}

}
