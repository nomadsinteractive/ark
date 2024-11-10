#include "core/base/state_action.h"

#include "core/base/state_action_strand.h"
#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

struct StateAction::StateActionExecute final : Runnable {
    StateActionExecute(sp<Stub> stub)
        : _stub(std::move(stub)) {
    }

    void run() override
    {
        _stub->_state_machine.doActionExecute({_stub});
    }

    sp<Stub> _stub;
};

struct StateAction::StateActionActivate final : Runnable {
    StateActionActivate(sp<Stub> stub)
        : _stub(std::move(stub)) {
    }

    void run() override
    {
        _stub->_state_machine.doActionActivate({_stub});
    }

    sp<Stub> _stub;
};

struct StateAction::StateActionDeactivate final : Runnable {
    StateActionDeactivate(sp<Stub> stub)
        : _stub(std::move(stub)) {
    }

    void run() override
    {
        _stub->_state_machine.doActionDeactivate({_stub});
    }

    sp<Stub> _stub;
};

StateAction::StateAction(StateMachine& stateMachine, sp<StateActionStrand> strand, sp<Runnable> onExecute, sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _stub(sp<Stub>::make(Stub{stateMachine, std::move(strand), std::move(onExecute), std::move(onActivate), std::move(onDeactivate)}))
{
}

StateAction::StateAction(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

const sp<StateActionStrand>& StateAction::strand() const
{
    return _stub->_strand;
}

const sp<State>& StateAction::start() const
{
    return _stub->_strand->_start;
}

const sp<State>& StateAction::end() const
{
    return _stub->_strand->_end;
}

const sp<Runnable>& StateAction::onExecute() const
{
    return _stub->_on_execute;
}

void StateAction::setOnExecute(sp<Runnable> onExecute)
{
    _stub->_on_execute = std::move(onExecute);
}

const sp<Runnable>& StateAction::onActivate() const
{
    return _stub->_on_activate;
}

void StateAction::setOnActivate(sp<Runnable> onActivate)
{
    _stub->_on_activate = std::move(onActivate);
}

const sp<Runnable>& StateAction::onDeactivate() const
{
    return _stub->_on_deactivate;
}

void StateAction::setOnDeactivate(sp<Runnable> onDeactivate)
{
    _stub->_on_deactivate = std::move(onDeactivate);
}

const sp<Runnable>& StateAction::execute()
{
    if(!_execute)
        _execute = sp<Runnable>::make<StateActionExecute>(_stub);
    return _execute;
}

const sp<Runnable>& StateAction::activate()
{
    if(!_activate)
        _activate = sp<Runnable>::make<StateActionActivate>(_stub);
    return _activate;
}

const sp<Runnable>& StateAction::deactivate()
{
    if(!_deactivate)
        _deactivate = sp<Runnable>::make<StateActionDeactivate>(_stub);
    return _deactivate;
}

}
