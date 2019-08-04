#ifndef ARK_CORE_BASE_COMMAND_H_
#define ARK_CORE_BASE_COMMAND_H_

#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state_machine.h"
#include "core/types/weak_ptr.h"
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
    Command(uint32_t id, const WeakPtr<StateMachine::Stub>& stateMachine, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive, uint32_t category);

    uint32_t id() const;

//  [[script::bindings::property]]
    bool active() const;

//  [[script::bindings::auto]]
    void execute() const;
//  [[script::bindings::auto]]
    void terminate() const;

    const sp<Runnable>& onActive() const;
    const sp<Runnable>& onDeactive() const;

    uint32_t category() const;
    bool conflicts(const Command& other) const;

    State state() const;
    void setState(State state);

private:
    uint32_t _id;
    WeakPtr<StateMachine::Stub> _state_machine;
    sp<Runnable> _on_active;
    sp<Runnable> _on_deactive;
    uint32_t _category;

    State _state;
};

}

#endif
