#include "core/base/state_machine.h"

#include "core/base/command.h"

namespace ark {

StateMachine::StateMachine()
    : _stub(sp<Stub>::make())
{
}

sp<Command> StateMachine::addCommand()
{
    return sp<Command>::make(++_stub->_new_command_id, _stub);
}

void StateMachine::push(const sp<State>& state)
{
    _stub->_states.push(state);
}

void StateMachine::pop()
{
    _stub->_states.pop();
}

const sp<State>& StateMachine::Stub::top() const
{
    if(_states.size() == 0)
        return sp<State>::null();

    return _states.top();
}

void StateMachine::Stub::activate(const Command& command)
{
    const sp<State>& current = top();
    DWARN(current, "Cannot activate Command(%d), StateMachine is empty", command.id());
    current->activate(command);
}

void StateMachine::Stub::deactivate(const Command& command)
{
    const sp<State>& current = top();
    DWARN(current, "Cannot deactivate Command(%d), StateMachine is empty", command.id());
    current->deactivate(command);
}

}
