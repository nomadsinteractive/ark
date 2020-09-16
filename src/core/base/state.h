#ifndef ARK_CORE_BASE_STATE_H_
#define ARK_CORE_BASE_STATE_H_

#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/command.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API State {
public:
//  [[script::bindings::enumeration]]
    enum StateFlag {
        STATE_FLAG_AUTO_DEACTIVATE = 1,
        STATE_FLAG_DEFAULT = 0
    };

public:
    State() = default;
    State(StateMachine& stateMachine, const sp<Runnable>& onActive, State::StateFlag flag = State::STATE_FLAG_DEFAULT);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(State);

    bool operator ==(const State& other) const;
    bool operator !=(const State& other) const;
    operator bool() const;

//  [[script::bindings::auto]]
    void activate() const;

    void execute(const Command& command) const;
    void terminate(const Command& command) const;

//  [[script::bindings::auto]]
    void linkCommand(const sp<Command>& command, const sp<Runnable>& onActive = nullptr, const sp<Runnable>& onDeactive = nullptr) const;

    void refreshState() const;

private:

    void deactivate() const;

    struct Stub {
        Stub(StateMachine& stateMachine, const sp<Runnable>& onActive, State::StateFlag flag = State::STATE_FLAG_DEFAULT);

        StateMachine& _state_machine;
        sp<Runnable> _on_active;
        StateFlag _flag;

        bool _activated;
        std::unordered_map<uint32_t, sp<Command>> _commands;

        const sp<Command>& getCommand(uint32_t commandId) const;
        int32_t resolveConflicts(const Command& command, Command::State state, Command::State toState) const;
    };

private:
    sp<Stub> _stub;
};

}

#endif
