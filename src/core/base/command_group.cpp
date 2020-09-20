#include "core/base/command_group.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

CommandGroup::CommandGroup(const sp<Runnable>& onActivate, const sp<Runnable>& onDeactivate, uint32_t mask)
    : _on_activate(onActivate), _on_deactivate(onDeactivate), _mask(mask), _state_holder(sp<Command::StateHolder>::make(Command::STATE_DEACTIVATED))
{
}

uint32_t CommandGroup::mask() const
{
    return _mask;
}

const sp<Command::StateHolder>& CommandGroup::stateHolder() const
{
    return _state_holder;
}

}
