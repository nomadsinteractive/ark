#include "core/base/state_action.h"

#include "core/base/state_action_strand.h"
#include "core/base/state_machine.h"

namespace ark {

StateAction::StateAction(StateMachine& stateMachine, sp<StateActionStrand> strand, sp<Runnable> onExecute, sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _state_machine(stateMachine), _strand(std::move(strand)), _on_execute(std::move(onExecute)), _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate))
{
    _strand->_actions.push_back(this);
}

const sp<StateActionStrand>& StateAction::strand() const
{
    return _strand;
}

const sp<State>& StateAction::start() const
{
    return _strand->_start;
}

const sp<State>& StateAction::end() const
{
    return _strand->_end;
}

const sp<Runnable>& StateAction::onExecute() const
{
    return _on_execute;
}

void StateAction::setOnExecute(sp<Runnable> onExecute)
{
    _on_execute = std::move(onExecute);
}

const sp<Runnable>& StateAction::onActivate() const
{
    return _on_activate;
}

void StateAction::setOnActivate(sp<Runnable> onActivate)
{
    _on_activate = std::move(onActivate);
}

const sp<Runnable>& StateAction::onDeactivate() const
{
    return _on_deactivate;
}

void StateAction::setOnDeactivate(sp<Runnable> onDeactivate)
{
    _on_deactivate = std::move(onDeactivate);
}

void StateAction::execute() const
{
    _state_machine.doActionExecute(*this);
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
