#include "core/base/command.h"

#include "core/base/command_group.h"
#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

Command::Command(StateMachine& stateMachine, sp<Runnable> onActivate, sp<CommandGroup> commandGroup)
    : _state_machine(stateMachine), _on_activate(std::move(onActivate)), _command_group(std::move(commandGroup)), _state_holder(_command_group ? _command_group->stateHolder() : sp<StateHolder>::make(STATE_DEACTIVATED))
{
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
    _state_holder->setState(state);
    if(state == STATE_ACTIVATED && _on_activate)
        _on_activate->run();
}

Command::StateHolder::StateHolder(Command::State state)
    : _state(state)
{
}

Command::State Command::StateHolder::state() const
{
    return _state;
}

void Command::StateHolder::setState(Command::State state)
{
    _state = state;
}

}
