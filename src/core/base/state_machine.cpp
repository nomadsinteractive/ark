#include "core/base/state_machine.h"

#include "core/base/state_action.h"
#include "core/base/state_action_strand.h"
#include "core/base/state.h"
#include "core/inf/runnable.h"

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

void StateMachine::doActionExecute(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.start()); iter != _active_states.end())
    {
        if(action._on_execute)
            action._on_execute->run();

        action.start()->doDeactivate();
        _active_states.erase(iter);

        doActive(action.end());
    }
}

void StateMachine::doActionActive(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.start()); iter != _active_states.end())
    {
        if(action._on_activate)
            action._on_activate->run();

        action._strand->doActionActive(action);
        doActive(action.end());
    }
}

void StateMachine::doActionDeactive(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.end()); iter != _active_states.end())
    {
        if(action._on_deactivate)
            action._on_deactivate->run();

        if(const StateAction* nextAction = action._strand->doActionDeactive(action))
        {
            if(nextAction->_on_activate)
                nextAction->_on_activate->run();

            nextAction->end()->doActivate();
        }
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
