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

bool BooleanByTimeout::update(uint32_t tick)
{
    return _duration->update(tick);
}

}
