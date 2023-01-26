#include "core/base/command.h"

#include "core/base/command_group.h"
#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

Command::Command(StateMachine& stateMachine, sp<Runnable> onActivate, sp<Runnable> onDeactivate, sp<CommandGroup> commandGroup)
    : _state_machine(stateMachine), _command_group(std::move(commandGroup)), _state_holder(sp<StateHolder>::make(std::move(onActivate), std::move(onDeactivate)))
{
    if(_command_group)
        _command_group->_commands.push_back(this);
}

void Command::activate()
{
    _state_machine.activateCommand(*this);
}

void Command::deactivate()
{
    _state_machine.deactivateCommand(*this);
}

uint32_t Command::mask() const
{
    return _command_group ? _command_group->mask() : 0;
}

bool Command::conflicts(const Command& other) const
{
    return (mask() & other.mask()) != 0;
}

Command::State Command::state() const
{
    return _state_holder->state();
}

void Command::setState(Command::State state)
{
    if(state == STATE_ACTIVATED && _command_group->stateHolder()->state() != state)
        _command_group->activate();
    _state_holder->setState(state);
}

const sp<CommandGroup>& Command::commandGroup() const
{
    return _command_group;
}

Command::StateHolder::StateHolder(sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _state(STATE_DEACTIVATED), _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate))
{
}

Command::State Command::StateHolder::state() const
{
    return _state;
}

void Command::StateHolder::setState(Command::State state)
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
