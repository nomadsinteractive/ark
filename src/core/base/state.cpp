#include "core/base/state.h"

#include "core/base/state_action.h"
#include "core/base/state_action_strand.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

namespace ark {

State::State(sp<Runnable> onActive, sp<Runnable> onDeactivate)
    : _on_active(std::move(onActive)), _on_deactive(std::move(onDeactivate)), _active(false)
{
}

bool State::active() const
{
    return _active;
}

void State::doActivate()
{
    if(_on_active)
        _on_active->run();
    _active = true;
}

void State::doDeactivate()
{
    CHECK_WARN(_active, "State is not active");
    if(_on_deactive && _active)
        _on_deactive->run();
    _active = false;
}

}
