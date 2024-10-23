#include "core/base/state_action_group.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

StateActionGroup::StateActionGroup(uint32_t mask, const sp<Runnable>& onActivate, const sp<Runnable>& onDeactivate)
    : _mask(mask)
{
}

uint32_t StateActionGroup::mask() const
{
    return _mask;
}

void StateActionGroup::activate()
{
}

void StateActionGroup::deactivate()
{
}

}
