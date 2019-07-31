#include "core/base/state.h"

#include "core/base/command.h"
#include "core/inf/runnable.h"

namespace ark {

State::State(const WeakPtr<StateMachine::Stub>& stateMachine, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive, State::StateFlag flag)
    : _stub(sp<Stub>::make(stateMachine, onActive, onDeactive, flag))
{
}

bool State::operator ==(const State& other) const
{
    return _stub == other._stub;
}

bool State::operator !=(const State& other) const
{
    return _stub != other._stub;
}

ark::State::operator bool() const
{
    return static_cast<bool>(_stub);
}

bool State::active() const
{
    return _stub->_state == Command::STATE_ACTIVATED;
}

void State::activate() const
{
    _stub->_state = Command::STATE_ACTIVATED;
    if(_stub->_on_active)
        _stub->_on_active->run();
}

void State::deactivate() const
{
    _stub->_state = Command::STATE_DEACTIVATED;
    if(_stub->_on_deactive)
        _stub->_on_deactive->run();
}

void State::linkCommand(const sp<Command>& command, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive) const
{
    DCHECK(_stub->_commands.find(command->id()) == _stub->_commands.end(), "Command has been linked to this state already");
    _stub->_commands[command->id()] = sp<CommandWithHandlers>::make(command, onActive ? onActive : command->onActive(), onDeactive ? onDeactive : command->onDeactive());
}

void State::refreshState() const
{
    if(_stub->_flag & STATE_FLAG_AUTO_DEACTIVATE)
    {
        for(const auto& i : _stub->_commands)
        {
            if(i.second->state() == Command::STATE_ACTIVATED)
                return;
        }
        _stub->_state_machine.ensure()->deactivate(*this);
    }
}

State::CommandWithHandlers::CommandWithHandlers(const sp<Command>& command, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
    : _command(command), _handlers{onActive, onDeactive, onDeactive}
{
}

Command::State State::CommandWithHandlers::state() const
{
    return _command->state();
}

void State::CommandWithHandlers::setState(Command::State state)
{
    DCHECK(state >= 0 && state < Command::STATE_COUNT, "Illegal state, stateId: %d", state);
    _command->setState(state);
    if(_handlers[state])
        _handlers[state]->run();
}

void State::execute(const Command& command) const
{
    const sp<CommandWithHandlers>& cws = _stub->getCommandWithHandlers(command.id());
    if(!cws)
        return;

    DCHECK(cws->state() != Command::STATE_ACTIVATED, "Illegal state, Command has been executed already");
    if(command.category())
        _stub->resolveConflicts(command, Command::STATE_ACTIVATED, Command::STATE_SUPPRESSED);
    cws->setState(Command::STATE_ACTIVATED);
}

void State::terminate(const Command& command) const
{
    const sp<CommandWithHandlers>& cws = _stub->getCommandWithHandlers(command.id());
    if(!cws)
        return;

    DCHECK(cws->state() != Command::STATE_DEACTIVATED, "Illegal state, Command has been terminated already");
    if(command.category())
    {
        cws->_command->setState(Command::STATE_DEACTIVATED);
        if(_stub->resolveConflicts(command, Command::STATE_SUPPRESSED, Command::STATE_ACTIVATED) == 0)
           cws->setState(Command::STATE_DEACTIVATED);
    }
    else
        cws->setState(Command::STATE_DEACTIVATED);

    refreshState();
}

const sp<State::CommandWithHandlers>& State::Stub::getCommandWithHandlers(uint32_t commandId) const
{
    const auto iter = _commands.find(commandId);
    return iter != _commands.end() ? iter->second : sp<State::CommandWithHandlers>::null();
}

int32_t State::Stub::resolveConflicts(const Command& command, Command::State state, Command::State toState) const
{
    int32_t count = 0;
    for(auto iter = _commands.begin(); iter != _commands.end(); ++iter)
    {
        const sp<State::CommandWithHandlers>& i = iter->second;
        if(iter->first != command.id() && i->_command->conflicts(command))
        {
            if(i->state() == state)
                i->setState(toState);
            if(i->state() == toState)
                ++count;
        }
    }
    return count;
}

State::Stub::Stub(const WeakPtr<StateMachine::Stub>& stateMachine, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive, State::StateFlag flag)
    : _state_machine(stateMachine), _on_active(onActive), _on_deactive(onDeactive), _flag(flag), _state(Command::STATE_DEACTIVATED)
{
}

}
