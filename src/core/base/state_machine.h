#ifndef ARK_CORE_BASE_STATE_MACHINE_H_
#define ARK_CORE_BASE_STATE_MACHINE_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateMachine {
public:
//  [[script::bindings::auto]]
    StateMachine();

//  [[script::bindings::auto]]
    sp<Command> addCommand(const sp<Runnable>& onActive = nullptr, uint32_t category = 0);

//  [[script::bindings::auto]]
    sp<State> addState(const sp<Runnable>& onActivate = nullptr, int32_t flag = 0);

    const std::vector<State>& states() const;

private:
    uint32_t _command_id_base;

    std::vector<State> _states;

    friend class Command;
    friend class State;
};

};

#endif
