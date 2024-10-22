#include "core/base/state_action.h"

#include "core/base/state_action_group.h"
#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

StateAction::StateAction(StateMachine& stateMachine, sp<Runnable> onActivate, sp<Runnable> onDeactivate, sp<StateActionGroup> commandGroup)
    : _state_machine(stateMachine), _command_group(std::move(commandGroup)), _state_holder(sp<StateHolder>::make(std::move(onActivate), std::move(onDeactivate)))
{
    if(_command_group)
        _command_group->_commands.push_back(this);
}

void StateAction::activate()
{
    _state_machine.activateCommand(*this);
}

void StateAction::deactivate()
{
    _state_machine.deactivateCommand(*this);
}

uint32_t StateAction::mask() const
{
    return _command_group ? _command_group->mask() : 0;
}

bool StateAction::conflicts(const StateAction& other) const
{
    return (mask() & other.mask()) != 0;
}

StateAction::State StateAction::state() const
{
    return _state_holder->state();
}

void StateAction::setState(StateAction::State state)
{
    if(state == STATE_ACTIVATED && _command_group->stateHolder()->state() != state)
        _command_group->activate();
    _state_holder->setState(state);
}

const sp<StateActionGroup>& StateAction::commandGroup() const
{
    return _command_group;
}

StateAction::StateHolder::StateHolder(sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _state(STATE_DEACTIVATED), _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate))
{
}

StateAction::State StateAction::StateHolder::state() const
{
    return _state;
}

void StateAction::StateHolder::setState(StateAction::State state)
{
    if(_state != state)
    {
        _state = state;
        switch(_state)
        {
            case STATE_ACTIVATED:
                if(_on_activate)
                    _on_activate->run();
            break;
            case STATE_DEACTIVATED:
                if(_on_deactivate)
                    _on_deactivate->run();
            break;
            default:
            break;
        }
    }
}

}
