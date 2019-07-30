#ifndef ARK_CORE_BASE_COMMAND_H_
#define ARK_CORE_BASE_COMMAND_H_

#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state_machine.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Command {
public:
    Command(uint32_t id, const sp<StateMachine::Stub>& stateMachine);

//  [[script::bindings::property]]
    uint32_t id() const;

//  [[script::bindings::auto]]
    void activate() const;
//  [[script::bindings::auto]]
    void deactivate() const;

private:
    uint32_t _id;
    sp<StateMachine::Stub> _state_machine;

    friend class State;
};

}

#endif
