#include "core/base/command.h"

#include "core/inf/runnable.h"

namespace ark {

Command::Command(uint32_t id, const sp<StateMachine::Stub>& stateMachine)
    : _id(id), _state_machine(stateMachine)
{
}

uint32_t Command::id() const
{
    return _id;
}

void Command::activate() const
{
    _state_machine->activate(*this);
}

void Command::deactivate() const
{
    _state_machine->deactivate(*this);
}

}
