#include "core/base/command.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

Command::Command(StateMachine& stateMachine, const sp<Runnable>& onActive, uint32_t mask)
    : _state_machine(stateMachine), _on_active(onActive), _mask(mask), _state(STATE_DEACTIVATED)
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
    return _mask;
}

bool Command::conflicts(const Command& other) const
{
    return (_mask & other._mask) != 0;
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
