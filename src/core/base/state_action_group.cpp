#include "core/base/state_action_group.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

StateActionGroup::StateActionGroup(uint32_t mask, const sp<Runnable>& onActivate, const sp<Runnable>& onDeactivate)
    : _mask(mask), _state_holder(sp<StateAction::StateHolder>::make(onActivate, onDeactivate))
{
}

uint32_t StateActionGroup::mask() const
{
    return _mask;
}

void StateActionGroup::activate()
{
    _state_holder->setState(StateAction::STATE_ACTIVATED);
}

void StateActionGroup::deactivate()
{
    _state_holder->setState(StateAction::STATE_DEACTIVATED);
}

int32_t StateActionGroup::resolveConflicts(const StateAction& command, StateAction::State state, StateAction::State toState) const
{
    int32_t count = 0;
    for(StateAction* i : _commands)
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

const sp<StateAction::StateHolder>& StateActionGroup::stateHolder() const
{
    return _state_holder;
}

}
