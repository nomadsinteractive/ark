#include "core/base/state.h"

#include "core/base/command.h"
#include "core/inf/runnable.h"

namespace ark {

State::State(State::StateFlag flag)
    : _exclusive(flag == STATE_FLAG_EXCLUSIVE)
{
}

void State::linkCommand(const Command& command, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
{
    DCHECK(_commands.find(command._id) == _commands.end(), "Command has been linked to this state already");
    _commands[command._id] = sp<CommandWithState>::make(onActive, onDeactive);
}

void State::execute(const Command& command, State::CommandState commandState)
{
    const auto iter = _commands.find(command._id);
    DCHECK(iter != _commands.end(), "Command does not belong to this State");
    iter->second->execute(commandState);
}

State::CommandWithState::CommandWithState(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
    : _state(COMMAND_STATE_DEACTIVATED), _handlers{onActive, onDeactive, onDeactive}
{
}

void State::CommandWithState::execute(CommandState state)
{
    DCHECK(state >= 0 && state < COMMAND_STATE_COUNT, "Illegal state, stateId: %d", state);
    _state = state;
    if(_handlers[_state])
        _handlers[_state]->run();
}

void State::activate(const Command& command)
{
    const sp<CommandWithState>& cws = ensureCommandWithState(command._id);
    DCHECK(cws->_state != State::COMMAND_STATE_ACTIVATED, "Illegal state, Command has been already activated");
    if(_exclusive)
        transfer(command._id, State::COMMAND_STATE_ACTIVATED, State::COMMAND_STATE_PAUSED);
    cws->execute(State::COMMAND_STATE_ACTIVATED);
}

void State::deactivate(const Command& command)
{
    const sp<CommandWithState>& cws = ensureCommandWithState(command._id);
    DCHECK(cws->_state != State::COMMAND_STATE_DEACTIVATED, "Illegal state, Command has been already deactivated");
    if(_exclusive)
    {
        cws->_state = State::COMMAND_STATE_DEACTIVATED;
        if(transfer(command._id, State::COMMAND_STATE_PAUSED, State::COMMAND_STATE_ACTIVATED) == 0)
           cws->execute(State::COMMAND_STATE_DEACTIVATED);
    }
    else
        cws->execute(State::COMMAND_STATE_DEACTIVATED);
}

int32_t State::transfer(uint32_t commandId, State::CommandState state, State::CommandState toState)
{
    int32_t count = 0;
    for(auto iter = _commands.begin(); iter != _commands.end(); ++iter)
    {
        const sp<State::CommandWithState>& i = iter->second;
        if(iter->first != commandId)
        {
            if(i->_state == state)
                i->execute(toState);
            if(i->_state == toState)
                ++count;
        }
    }
    return count;
}

const sp<State::CommandWithState>& State::ensureCommandWithState(uint32_t commandId) const
{
    const auto iter = _commands.find(commandId);
    DCHECK(iter != _commands.end(), "Command does not belong to this State");
    return iter->second;
}

}
