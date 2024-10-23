#include "core/base/state.h"

#include "core/base/state_action.h"
#include "core/base/state_action_group.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

namespace ark {

State::State(sp<Runnable> onActive, sp<Runnable> onDeactivate)
    : _active(false), _on_active(std::move(onActive)), _on_deactive(std::move(onDeactivate))
{
}

bool State::active() const
{
    return _active;
}

sp<StateAction> State::createAction(const sp<State>& nextState, sp<StateActionGroup> strand)
{
    sp<StateAction> action = sp<StateAction>::make(*this, *nextState, std::move(strand));
    _actions.push_back(action);
    return action;
}

void State::doActivate()
{
    if(_on_active && !_active)
        _on_active->run();
    _active = true;
}

void State::doDeactivate()
{
    if(_on_deactive && _active)
        _on_deactive->run();
    _active = false;
}

}
