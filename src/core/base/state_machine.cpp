#include "core/base/state_machine.h"

#include "core/base/command.h"
#include "core/base/command_group.h"
#include "core/base/state.h"

namespace ark {

StateMachine::StateMachine()
    : _active_state(nullptr)
{
}

sp<Command> StateMachine::addCommand(sp<Runnable> onActivate, sp<Runnable> onDeactivate, sp<CommandGroup> commandGroup)
{
    sp<Command> cmd = sp<Command>::make(*this, std::move(onActivate), std::move(onDeactivate), std::move(commandGroup));
    _commands.push_back(cmd);
    return cmd;
}

sp<State> StateMachine::addState(const sp<Runnable>& onActivate, const sp<State>& fallback)
{
    sp<State> state = sp<State>::make(*this, onActivate, fallback.get());
    _states.push_back(state);
    return state;
}

void StateMachine::start(State& entry)
{
    _active_state = &entry;
    _active_state->activate();
}

void StateMachine::transit(State& next)
{
    CHECK(_active_state, "Statemachine has not active state(call start first)");
    _active_state->deactivate();
    start(next);
}

void StateMachine::activateCommand(Command& command)
{
    CHECK(command.state() != Command::STATE_ACTIVATED, "Illegal state, Command has been executed already");
    command.setState(Command::STATE_ACTIVATED);
    if(command.commandGroup())
        command.commandGroup()->resolveConflicts(command, Command::STATE_ACTIVATED, Command::STATE_SUPPRESSED);
    if(command.mask())
    {
        ASSERT(_active_state);
        _active_state->resolveConflicts(command, Command::STATE_ACTIVATED, Command::STATE_SUPPRESSED);
    }
}

void StateMachine::deactivateCommand(Command& command)
{
    CHECK(command.state() != Command::STATE_DEACTIVATED, "Illegal state, Command has been terminated already");
    command.setState(Command::STATE_DEACTIVATED);
    if(command.commandGroup() && command.commandGroup()->resolveConflicts(command, Command::STATE_SUPPRESSED, Command::STATE_ACTIVATED) == 0)
        command.commandGroup()->deactivate();
    if(_active_state->resolveConflicts(command, Command::STATE_SUPPRESSED, Command::STATE_ACTIVATED) == 0 && _active_state->_fallback)
        transit(*_active_state->_fallback);
}

const std::vector<sp<State>>& StateMachine::states() const
{
    return _states;
}

}
