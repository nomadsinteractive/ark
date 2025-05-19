#include "core/base/state_machine.h"

#include "core/base/state_action.h"
#include "core/base/state_action_strand.h"
#include "core/base/state.h"
#include "core/inf/runnable.h"

namespace ark {

struct StateMachine::Stub {

    Vector<sp<State>> _states;

};

StateMachine::StateMachine(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

StateMachine::StateMachine()
    : _stub(sp<Stub>::make())
{
}

void StateMachine::addState(sp<State> state) const
{
    _stub->_states.push_back(std::move(state));
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
