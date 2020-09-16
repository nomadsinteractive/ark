#include "core/base/command.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

Command::Command(StateMachine& stateMachine, uint32_t id, const sp<Runnable>& onActive, uint32_t category)
    : _state_machine(stateMachine), _id(id), _on_active(onActive), _category(category), _state(STATE_DEACTIVATED)
{
}

uint32_t Command::id() const
{
    return _id;
}

bool Command::active() const
{
    return _state != STATE_DEACTIVATED;
}

void Command::execute()
{
    setState(STATE_ACTIVATED);
}

void Command::terminate()
{
    _state = STATE_DEACTIVATED;
}

uint32_t Command::category() const
{
    return _category;
}

bool Command::conflicts(const Command& other) const
{
    return (_category & other._category) != 0;
}

Command::State Command::state() const
{
    return _state;
}

void Command::setState(Command::State state)
{
    _state = state;
    if(_state == STATE_ACTIVATED && _on_active)
        _on_active->run();
}

}
