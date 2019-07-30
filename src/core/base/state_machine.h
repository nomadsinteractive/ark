#ifndef ARK_CORE_BASE_STATE_MACHINE_H_
#define ARK_CORE_BASE_STATE_MACHINE_H_

#include <stack>

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
    sp<Command> addCommand();
//  [[script::bindings::auto]]
    void push(const sp<State>& state);
//  [[script::bindings::auto]]
    void pop();

private:
    struct Stub {

        const sp<State>& top() const;

        void activate(const Command& command);
        void deactivate(const Command& command);

        uint32_t _new_command_id;
        std::stack<sp<State>> _states;
    };

private:
    sp<Stub> _stub;

    friend class Command;
};

};

#endif
