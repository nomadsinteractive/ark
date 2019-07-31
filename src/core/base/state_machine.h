#ifndef ARK_CORE_BASE_STATE_MACHINE_H_
#define ARK_CORE_BASE_STATE_MACHINE_H_

#include <list>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateMachine {
public:
//  [[script::bindings::auto]]
    StateMachine();

//  [[script::bindings::auto]]
    sp<Command> addCommand(const sp<Runnable>& onActive = nullptr, const sp<Runnable>& onDeactive = nullptr);

//  [[script::bindings::auto]]
    sp<State> addState(const sp<Runnable>& onActive = nullptr, const sp<Runnable>& onDeactive = nullptr, int32_t flag = 0);

//  [[script::bindings::auto]]
    void activate(const State& state);
//  [[script::bindings::auto]]
    void deactivate(const State& state);

private:
    struct Stub {

        void activate(const State& state);
        void deactivate(const State& state);

        State front() const;
        void pop();

        void execute(const Command& command);
        void terminate(const Command& command);

        uint32_t _new_command_id;
        std::list<State> _states;
    };

private:
    sp<Stub> _stub;

    friend class Command;
    friend class State;
};

};

#endif
