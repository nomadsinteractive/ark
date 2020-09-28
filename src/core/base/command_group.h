#ifndef ARK_CORE_BASE_COMMAND_GROUP_H_
#define ARK_CORE_BASE_COMMAND_GROUP_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/command.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API CommandGroup {
public:
//  [[script::bindings::auto]]
    CommandGroup(uint32_t mask, const sp<Runnable>& onActivate = nullptr, const sp<Runnable>& onDeactivate = nullptr);

//  [[script::bindings::property]]
    uint32_t mask() const;
    
    void activate();
    void deactivate();

    int32_t resolveConflicts(const Command& command, Command::State state, Command::State toState) const;

    const sp<Command::StateHolder>& stateHolder() const;

private:
    uint32_t _mask;
    
    sp<Command::StateHolder> _state_holder;

    std::vector<Command*> _commands;

    friend class Command;
    friend class State;
};

}

#endif
