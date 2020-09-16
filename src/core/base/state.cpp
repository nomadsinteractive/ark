#include "core/base/state.h"

#include "core/base/command.h"
#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

State::State(StateMachine& stateMachine, const sp<Runnable>& onActive, State::StateFlag flag)
    : _stub(sp<Stub>::make(stateMachine, onActive, flag))
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

void State::activate() const
{
    for(const State& i : _stub->_state_machine.states())
        if(i != *this && i._stub->_activated)
            i.deactivate();

    _stub->_activated = true;
    if(_stub->_on_active)
        _stub->_on_active->run();
}

void State::linkCommand(const sp<Command>& command, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive) const
{
    DCHECK(_stub->_commands.find(command->id()) == _stub->_commands.end(), "Command has been linked to this state already");
    _stub->_commands[command->id()] = command;
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
    }
}

void State::deactivate() const
{
    for(const auto& iter : _stub->_commands)
        iter.second->terminate();
}

void State::execute(const Command& command) const
{
    const sp<Command>& cws = _stub->getCommand(command.id());
    if(!cws)
        return;

    DCHECK(cws->state() != Command::STATE_ACTIVATED, "Illegal state, Command has been executed already");
    if(command.category())
        _stub->resolveConflicts(command, Command::STATE_ACTIVATED, Command::STATE_SUPPRESSED);
    cws->execute();
}

void State::terminate(const Command& command) const
{
    const sp<Command>& cws = _stub->getCommand(command.id());
    if(!cws)
        return;

    DCHECK(cws->state() != Command::STATE_DEACTIVATED, "Illegal state, Command has been terminated already");
    if(command.category())
        _stub->resolveConflicts(command, Command::STATE_SUPPRESSED, Command::STATE_ACTIVATED);
    cws->terminate();
}

const sp<Command>& State::Stub::getCommand(uint32_t commandId) const
{
    const auto iter = _commands.find(commandId);
    return iter != _commands.end() ? iter->second : sp<Command>::null();
}

int32_t State::Stub::resolveConflicts(const Command& command, Command::State state, Command::State toState) const
{
    int32_t count = 0;
    for(auto iter = _commands.begin(); iter != _commands.end(); ++iter)
    {
        const sp<Command>& i = iter->second;
        if(iter->first != command.id() && i->conflicts(command))
        {
            if(i->state() == state)
                i->setState(toState);
            if(i->state() == toState)
                ++count;
        }
    }
    return count;
}

State::Stub::Stub(StateMachine& stateMachine, const sp<Runnable>& onActive, State::StateFlag flag)
    : _state_machine(stateMachine), _on_active(onActive),_flag(flag), _activated(false)
{
}

}
