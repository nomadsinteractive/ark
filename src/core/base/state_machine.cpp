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
            i->deactivate();

    _active_states.clear();
    doActive(std::move(state));
}

void StateMachine::doActionExecute(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.start()); iter != _active_states.end())
    {
        if(action._stub->_on_execute)
            action._stub->_on_execute->run();

        action.start()->deactivate();
        _active_states.erase(iter);

        doActive(action.end());
    }
}

void StateMachine::doActionActivate(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.start()); iter != _active_states.end())
    {
        if(action._stub->_on_activate)
            action._stub->_on_activate->run();

        action._stub->_strand->doActionActive(action);
        doActive(action.end());
    }
}

void StateMachine::doActionDeactivate(const StateAction& action)
{
    if(const auto iter = std::find(_active_states.begin(), _active_states.end(), action.end()); iter != _active_states.end())
    {
        if(action._stub->_on_deactivate)
            action._stub->_on_deactivate->run();

        if(action._stub->_strand->doActionDeactive(action))
        {
            action.end()->deactivate();
            _active_states.erase(iter);
            doActive(action.start());
        }
    }
}

void StateMachine::doActive(sp<State> state)
{
    state->activate();
    _active_states.push_back(std::move(state));
}

}
