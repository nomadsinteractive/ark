#include "core/base/command.h"

#include "core/inf/runnable.h"

namespace ark {

Command::Command(uint32_t id, const WeakPtr<StateMachine::Stub>& stateMachine, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
    : _id(id), _state_machine(stateMachine), _on_active(onActive), _on_deactive(onDeactive), _state(STATE_DEACTIVATED)
{
}

uint32_t Command::id() const
{
    return _id;
}

void Command::execute() const
{
    _state_machine.ensure()->execute(*this);
}

void Command::terminate() const
{
    _state_machine.ensure()->terminate(*this);
}

const sp<Runnable>& Command::onActive() const
{
    return _on_active;
}

const sp<Runnable>& Command::onDeactive() const
{
    return _on_deactive;
}

Command::State Command::state() const
{
    return _state;
}

void Command::setState(Command::State state)
{
    _state = state;
}

}
