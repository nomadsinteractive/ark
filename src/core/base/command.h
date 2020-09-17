#ifndef ARK_CORE_BASE_COMMAND_H_
#define ARK_CORE_BASE_COMMAND_H_

#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/runnable.h"
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

public:
    Command(StateMachine& stateMachine, const sp<Runnable>& onActive, uint32_t mask);

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
    sp<Runnable> _on_active;
    uint32_t _mask;

    State _state;

};

}

#endif
