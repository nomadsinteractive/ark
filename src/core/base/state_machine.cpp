#include "core/base/state_machine.h"

#include "core/base/state_action.h"
#include "core/base/state_action_strand.h"
#include "core/base/state.h"

namespace ark {

StateMachine::StateMachine(sp<State> entry)
{
    if(entry)
        doActive(std::move(entry));
}

void StateMachine::reset(sp<State> state)
{
    for(const sp<State>& i : _active_states)
        if(i != state)
            i->doDeactivate();

    _active_states.clear();
    doActive(std::move(state));
}

void StateMachine::doActionTransit(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.start()); iter != _active_states.end())
    {
        action.start()->doDeactivate();
        _active_states.erase(iter);

        doActive(action.end());
    }
}

void StateMachine::doActionActive(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.start()); iter != _active_states.end())
    {
        action._strand->doActionActive(action);
        doActive(action.end());
    }
}

void StateMachine::doActionDeactive(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.end()); iter != _active_states.end())
    {
        if(const StateAction* nextAction = action._strand->doActionDeactive(action))
            nextAction->end()->doActivate();
        else
        {
            action.end()->doDeactivate();
            _active_states.erase(iter);
            doActive(action.start());
        }
    }
}

void StateMachine::doActive(sp<State> state)
{
    state->doActivate();
    _active_states.push_back(std::move(state));
}

}
