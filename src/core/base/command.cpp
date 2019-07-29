#include "core/base/command.h"

#include "core/base/command_set.h"
#include "core/inf/runnable.h"

namespace ark {

Command::Command(const sp<CommandSet>& commandSet, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
    : _command_set(commandSet), _stub(sp<Stub>::make(onActive, onDeactive))
{
}

Command::State Command::state() const
{
    return _stub->_state;
}

void Command::activate() const
{
    _command_set->activate(_stub);
}

void Command::deactivate() const
{
   _command_set->deactivate(_stub);
}

Command::Stub::Stub(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
    : _state(STATE_DEACTIVATED), _handlers{onActive, onDeactive, onDeactive}
{
}

void Command::Stub::execute(Command::State state)
{
    DCHECK(state >= 0 && state < STATE_COUNT, "Illegal state, stateId: %d", state);
    _state = state;
    if(_handlers[_state])
        _handlers[_state]->run();
}

}
