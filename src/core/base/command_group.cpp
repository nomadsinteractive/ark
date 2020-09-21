#include "core/base/command_group.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

CommandGroup::CommandGroup(uint32_t mask, const sp<Runnable>& onActivate, const sp<Runnable>& onDeactivate)
    : _mask(mask), _state_holder(sp<Command::StateHolder>::make(onActivate, onDeactivate))
{
}

uint32_t CommandGroup::mask() const
{
    return _mask;
}

void CommandGroup::activate()
{
    _state_holder->setState(Command::STATE_ACTIVATED);
}

void CommandGroup::deactivate()
{
    _state_holder->setState(Command::STATE_DEACTIVATED);
}

int32_t CommandGroup::resolveConflicts(const Command& command, Command::State state, Command::State toState) const
{
    int32_t count = 0;
    for(Command* i : _commands)
    {
        if(i != &command)
        {
            if(i->state() == state)
                i->setState(toState);
            if(i->state() == toState)
                ++count;
        }
    }
    return count;
}

const sp<Command::StateHolder>& CommandGroup::stateHolder() const
{
    return _state_holder;
}

}
