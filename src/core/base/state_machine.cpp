#include "core/base/state_machine.h"

#include "core/base/command.h"
#include "core/base/state.h"

namespace ark {

StateMachine::StateMachine()
    : _command_id_base(0)
{
}

sp<Command> StateMachine::addCommand(const sp<Runnable>& onActive, uint32_t category)
{
    return sp<Command>::make(*this, ++_command_id_base, onActive, category);
}

sp<State> StateMachine::addState(const sp<Runnable>& onActivate, int32_t flag)
{
    sp<State> state = sp<State>::make(*this, onActivate, static_cast<State::StateFlag>(flag));
    _states.push_back(*state);
    return state;
}

const std::vector<State>& StateMachine::states() const
{
    return _states;
}

}
