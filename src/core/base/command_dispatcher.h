#ifndef ARK_CORE_BASE_COMMAND_DISPATCHER_H_
#define ARK_CORE_BASE_COMMAND_DISPATCHER_H_

#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/command.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API CommandDispatcher {
public:
//  [[script::bindings::enumeration]]
    enum GroupFlag {
        GROUP_FLAG_DEFAULT,
        GROUP_FLAG_EXCLUSIVE
    };

public:
//  [[script::bindings::auto]]
    CommandDispatcher(CommandDispatcher::GroupFlag groupFlag = CommandDispatcher::GROUP_FLAG_DEFAULT);

//  [[script::bindings::auto]]
    sp<Command> makeCommand(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive);

private:
    sp<CommandSet> _current;

    friend class Command;
};

}

#endif
