#include "core/base/command_set.h"

#include "core/base/command_dispatcher.h"
#include "core/inf/runnable.h"

namespace ark {

CommandSet::CommandSet(bool exclusive)
    : _exclusive(exclusive)
{
}

void CommandSet::activate(const sp<Command::Stub>& toActive)
{
    DCHECK(toActive->_state != Command::STATE_ACTIVATED, "Illegal state, Command has been already activated");
    if(_exclusive)
        transfer(Command::STATE_ACTIVATED, Command::STATE_PAUSED);
    _commands.insert(toActive);
    toActive->execute(Command::STATE_ACTIVATED);
}

void CommandSet::deactivate(const sp<Command::Stub>& toTerminate)
{
    const auto iter = _commands.find(toTerminate);
    DCHECK(iter != _commands.end(), "Command has not been activated");
    if(_exclusive)
    {
        toTerminate->_state = Command::STATE_DEACTIVATED;
        if(transfer(Command::STATE_PAUSED, Command::STATE_ACTIVATED) == 0)
           toTerminate->execute(Command::STATE_DEACTIVATED);
    }
    else
        toTerminate->execute(Command::STATE_DEACTIVATED);
}

int32_t CommandSet::transfer(Command::State state, Command::State toState)
{
    int32_t count = 0;
    for(auto iter = _commands.begin(); iter != _commands.end(); )
    {
        const sp<Command::Stub>& i = *iter;
        if(i->_state == state)
            i->execute(toState);
        if(i->_state == toState)
            ++count;
        if(i->_state == Command::STATE_DEACTIVATED)
        {
            iter = _commands.erase(iter);
            if(iter == _commands.end())
                break;
        }
        else
            ++iter;
    }
    return count;
}

}
