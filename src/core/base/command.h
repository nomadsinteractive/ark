#ifndef ARK_CORE_BASE_COMMAND_H_
#define ARK_CORE_BASE_COMMAND_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Command {
public:
    enum State {
        STATE_ACTIVATED,
        STATE_DEACTIVATED,
        STATE_SUPPRESSED,
        STATE_COUNT
    };

    class StateHolder {
    public:
        StateHolder(State state);

        State state() const;
        void setState(State state);

    private:
        State _state;
    };

public:
    Command(StateMachine& stateMachine, sp<Runnable> onActivate, sp<CommandGroup> commandGroup);

//  [[script::bindings::auto]]
    void activate();
//  [[script::bindings::auto]]
    void deactivate();

//  [[script::bindings::property]]
    uint32_t mask() const;

    bool conflicts(const Command& other) const;

    State state() const;
    void setState(State state);

private:
    StateMachine& _state_machine;
    sp<Runnable> _on_activate;
    sp<CommandGroup> _command_group;

    sp<StateHolder> _state_holder;

};

}

#endif
