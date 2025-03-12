#include "core/impl/boolean/boolean_by_timeout.h"

namespace ark {

BooleanByTimeout::BooleanByTimeout(sp<Numeric> duration, const float timeout, const bool timeoutValue)
    : _duration(std::move(duration)), _timeout(_duration->val() + timeout), _timeout_value(timeoutValue)
{
}

bool BooleanByTimeout::val()
{
    return _duration->val() > _timeout ? _timeout_value : !_timeout_value;
}

bool BooleanByTimeout::update(const uint64_t timestamp)
{
    return _duration->update(timestamp);
}

}
