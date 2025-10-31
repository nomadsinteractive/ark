#include "core/components/discarded.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Discarded::Discarded(bool discarded)
    : _discarded(sp<BooleanWrapper>::make(discarded))
{
}

Discarded::Discarded(sp<Boolean> discarded)
    : _discarded(discarded ? sp<BooleanWrapper>::make(std::move(discarded)) : sp<BooleanWrapper>::make(false))
{
}

bool Discarded::val()
{
    return _discarded->val();
}

bool Discarded::update(uint32_t tick)
{
    return _discarded->update(tick);
}

void Discarded::discard() const
{
    _discarded->set(true);
}

void Discarded::set(const bool discarded) const
{
    _discarded->set(discarded);
}

void Discarded::set(sp<Boolean> discarded) const
{
    _discarded->set(std::move(discarded));
}

}
