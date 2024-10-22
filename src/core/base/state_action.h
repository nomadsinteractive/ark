#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateAction {
public:
    enum State {
        STATE_ACTIVATED,
        STATE_DEACTIVATED,
        STATE_SUPPRESSED,
        STATE_COUNT
    };

    class StateHolder {
    public:
        StateHolder(sp<Runnable> onActivate, sp<Runnable> onDeactivate);

        State state() const;
        void setState(State state);

    private:
        State _state;

        sp<Runnable> _on_activate;
        sp<Runnable> _on_deactivate;
    };

public:
    StateAction(StateMachine& stateMachine, sp<Runnable> onActivate, sp<Runnable> onDeactivate, sp<StateActionGroup> commandGroup);

//  [[script::bindings::auto]]
    void activate();
//  [[script::bindings::auto]]
    void deactivate();

//  [[script::bindings::property]]
    uint32_t mask() const;

    bool conflicts(const StateAction& other) const;

    State state() const;
    void setState(State state);

    const sp<StateActionGroup>& commandGroup() const;

private:
    StateMachine& _state_machine;
    sp<StateActionGroup> _command_group;

    sp<StateHolder> _state_holder;

};

}
