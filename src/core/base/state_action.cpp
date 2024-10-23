#include "core/base/state_action.h"

#include "core/base/state_action_strand.h"
#include "core/base/state_machine.h"

namespace ark {

namespace {

sp<StateActionStrand> ensureStateActionStrand(const sp<State>& start, const sp<State>& end, const sp<StateActionStrand>& actionStrand)
{
    if(actionStrand)
    {
        CHECK((actionStrand->_start == nullptr || actionStrand->_start == start) && (actionStrand->_end == nullptr || actionStrand->_end == end), "All actions in Strand must have the same start and end states.");
        if(actionStrand->_start == nullptr)
            actionStrand->_start = start;
        if(actionStrand->_end == nullptr)
            actionStrand->_end = end;

        return actionStrand;
    }

    return sp<StateActionStrand>::make(start, end);
}

}

StateAction::StateAction(StateMachine& stateMachine, const sp<State>& start, const sp<State>& end, const sp<StateActionStrand>& actionStrand)
    : _state_machine(stateMachine), _strand(ensureStateActionStrand(start, end, actionStrand))
{
    _strand->_actions.push_back(this);
}

const sp<State>& StateAction::start() const
{
    return _strand->_start;
}

const sp<State>& StateAction::end() const
{
    return _strand->_end;
}

void StateAction::execute() const
{
    _state_machine.doActionTransit(*this);
}

void StateAction::activate() const
{
    _state_machine.doActionActive(*this);
}

void StateAction::deactivate() const
{
    _state_machine.doActionDeactive(*this);
}

}
