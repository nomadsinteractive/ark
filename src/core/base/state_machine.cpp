#include "core/base/state_machine.h"

#include "core/base/state_action.h"
#include "core/base/state_action_group.h"
#include "core/base/state.h"

namespace ark {

StateMachine::StateMachine(sp<State> entry)
    : _active_state(entry.get()), _states{std::move(entry)}
{
}

void StateMachine::addState(sp<State> state)
{
    _states.push_back(std::move(state));
}

}
