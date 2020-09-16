#ifndef ARK_CORE_BASE_COMMAND_H_
#define ARK_CORE_BASE_COMMAND_H_

#include <set>

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

public:
    Command(StateMachine& stateMachine, uint32_t id, const sp<Runnable>& onActive, uint32_t category);

    uint32_t id() const;

//  [[script::bindings::property]]
    bool active() const;

//  [[script::bindings::auto]]
    void execute();
//  [[script::bindings::auto]]
    void terminate();

//  [[script::bindings::property]]
    uint32_t category() const;

    bool conflicts(const Command& other) const;

    State state() const;
    void setState(State state);

private:
    StateMachine& _state_machine;
    uint32_t _id;
    sp<Runnable> _on_active;
    uint32_t _category;

    State _state;
};

}

#endif
