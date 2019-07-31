#include "core/base/state_machine.h"

#include "core/base/command.h"
#include "core/base/state.h"

namespace ark {

StateMachine::StateMachine()
    : _stub(sp<Stub>::make())
{
}

sp<Command> StateMachine::addCommand(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive, uint32_t category)
{
    return sp<Command>::make(++_stub->_new_command_id, _stub, onActive, onDeactive, category);
}

sp<State> StateMachine::addState(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive, int32_t flag)
{
    return sp<State>::make(_stub, onActive, onDeactive, static_cast<State::StateFlag>(flag));
}

void StateMachine::activate(const State& state)
{
    _stub->activate(state);
}

void StateMachine::deactivate(const State& state)
{
    _stub->deactivate(state);
}

void StateMachine::Stub::activate(const State& state)
{
    if(!state.active())
    {
        state.activate();
        _states.push_front(state);
    }
}

void StateMachine::Stub::deactivate(const State& state)
{
    if(state.active())
    {
        state.deactivate();
        if(_states.size() > 0 && _states.front() == state)
            pop();
    }
}

State StateMachine::Stub::front() const
{
    if(_states.size() == 0)
        return State();

    return _states.front();
}

void StateMachine::Stub::pop()
{
    do {
        _states.pop_front();
    } while(!_states.empty() && !_states.front().active());

    if(_states.size() > 0)
        _states.front().refreshState();

    if(_states.size() > 0)
        _states.front().activate();
}

void StateMachine::Stub::execute(const Command& command)
{
    const State top = front();
    DWARN(top, "Cannot execute Command(%d), StateMachine is empty", command.id());
    if(top)
        top.execute(command);
}

void StateMachine::Stub::terminate(const Command& command)
{
    const State top = front();
    DWARN(top, "Cannot terminate Command(%d), StateMachine is empty", command.id());
    if(top)
        top.terminate(command);
}

}
