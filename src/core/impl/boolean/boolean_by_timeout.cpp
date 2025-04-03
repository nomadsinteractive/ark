#include "core/impl/boolean/boolean_by_timeout.h"

namespace ark {

BooleanByTimeout::BooleanByTimeout(sp<Numeric> duration, const float timeout)
    : _duration(std::move(duration)), _timeout(_duration->val() + timeout)
{
}

bool BooleanByTimeout::val()
{
    return _duration->val() > _timeout;
}

bool BooleanByTimeout::update(const uint64_t timestamp)
{
    return _duration->update(timestamp);
}

}
