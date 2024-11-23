#include "core/base/state.h"

#include "core/base/state_action.h"
#include "core/base/state_action_strand.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

namespace ark {

State::State(sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate)), _active(false)
{
}

bool State::active() const
{
    return _active;
}

void State::doActivate()
{
    if(_on_activate)
        _on_activate->run();
    _active = true;
}

void State::doDeactivate()
{
    CHECK_WARN(_active, "State is not active");
    if(_on_deactivate && _active)
        _on_deactivate->run();
    _active = false;
}

}
