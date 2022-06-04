#include "core/base/state.h"

#include "core/base/command.h"
#include "core/base/command_group.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

namespace ark {

State::State(StateMachine& /*stateMachine*/, const sp<Runnable>& onActive, State* fallback)
    : _on_active(onActive), _fallback(fallback), _active(false)
{
}

bool State::active() const
{
    return _active;
}

void State::activate()
{
    if(_on_active && !_active)
        _on_active->run();
    _active = true;
}

void State::deactivate()
{
    _active = false;
}

void State::linkCommand(Command& command)
{
    DCHECK(_linked_commands.find(&command) == _linked_commands.end(), "Command has been linked to this state already");
    _linked_commands.insert(std::make_pair(&command, nullptr));
}

void State::linkCommandGroup(CommandGroup& commandGroup)
{
    for(Command* i : commandGroup._commands)
        linkCommand(*i);
}

int32_t State::resolveConflicts(const Command& command, Command::State state, Command::State toState) const
{
    int32_t count = 0;
    for(const auto& i : _linked_commands)
    {
        Command* cmd = i.first;
        if(cmd != &command)
        {
            if(cmd->conflicts(command) && cmd->state() == state)
                cmd->setState(toState);
            if(cmd->state() == toState)
                ++count;
        }
    }
    return count;
}

}
