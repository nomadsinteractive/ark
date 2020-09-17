#ifndef ARK_CORE_BASE_STATE_H_
#define ARK_CORE_BASE_STATE_H_

#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/command.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

namespace ark {

class ARK_API State {
public:
    State(StateMachine& stateMachine, const sp<Runnable>& onActive, State* fallback = nullptr);

//  [[script::bindings::property]]
    bool active() const;

    void activate();
    void deactivate();

    void activate(Command& command) const;
    void deactivate(Command& command) const;

//  [[script::bindings::auto]]
    void linkCommand(Command& command);

    int32_t resolveConflicts(const Command& command, Command::State state, Command::State toState) const;

private:
    sp<Runnable> _on_active;
    State* _fallback;
    bool _active;

    std::unordered_map<Command*, State*> _linked_commands;

    friend class StateMachine;
};

}

#endif
