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
    _activated_action_stack.push_back(&action);
}

const StateAction* StateActionStrand::doActionDeactive(const StateAction& action)
{
    if(const auto iter = std::find(_activated_action_stack.begin(), _activated_action_stack.end(), &action); iter != _activated_action_stack.end())
        _activated_action_stack.erase(iter);
    return _activated_action_stack.empty() ? nullptr : _activated_action_stack.back();
}

}
