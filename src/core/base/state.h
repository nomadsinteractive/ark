#ifndef ARK_CORE_BASE_STATE_H_
#define ARK_CORE_BASE_STATE_H_

#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API State {
public:
//  [[script::bindings::enumeration]]
    enum StateFlag {
        STATE_FLAG_DEFAULT,
        STATE_FLAG_EXCLUSIVE
    };

    enum CommandState {
        COMMAND_STATE_ACTIVATED,
        COMMAND_STATE_DEACTIVATED,
        COMMAND_STATE_PAUSED,
        COMMAND_STATE_COUNT
    };

public:
//  [[script::bindings::auto]]
    State(State::StateFlag flag = State::STATE_FLAG_DEFAULT);

    void activate(const Command& command);
    void deactivate(const Command& command);

    void execute(const Command& command, CommandState commandState);

//  [[script::bindings::auto]]
    void linkCommand(const Command& command, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive);

private:
    struct CommandWithState {
        CommandWithState(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive);

        void execute(CommandState state);

        CommandState _state;
        sp<Runnable> _handlers[COMMAND_STATE_COUNT];
    };

private:
    int32_t transfer(uint32_t commandId, State::CommandState state, State::CommandState toState);
    const sp<CommandWithState>& ensureCommandWithState(uint32_t commandId) const;

private:
    bool _exclusive;

    std::unordered_map<uint32_t, sp<CommandWithState>> _commands;

    friend class Command;
};

}

#endif
