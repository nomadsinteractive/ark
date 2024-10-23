#include "core/base/state_action.h"

#include "core/base/state_action_group.h"
#include "core/base/state_machine.h"
#include "core/inf/runnable.h"

namespace ark {

StateAction::StateAction(State& start, State& end, sp<StateActionGroup> actionStrand)
    : _start(start), _end(end), _action_strand(std::move(actionStrand))
{
}

void StateAction::execute()
{
}

void StateAction::activate()
{
}

void StateAction::deactivate()
{
}

}
