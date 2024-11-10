#include "core/base/state_action_strand.h"

#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

StateActionStrand::StateActionStrand(sp<State> start, sp<State> end)
    : _start(std::move(start)), _end(std::move(end))
{
}

void StateActionStrand::doActionActive(const StateAction& action)
{
    _activated_actions.push_back(action._stub);
}

bool StateActionStrand::doActionDeactive(const StateAction& action)
{
    if(const auto iter = std::find(_activated_actions.begin(), _activated_actions.end(), action._stub); iter != _activated_actions.end())
        _activated_actions.erase(iter);

    if(_activated_actions.empty())
        return true;

    const sp<StateAction::Stub>& nextActionStub = _activated_actions.front();
    if(nextActionStub->_on_activate)
        nextActionStub->_on_activate->run();
    return false;
}

}
