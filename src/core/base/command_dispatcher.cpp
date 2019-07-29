#include "core/base/command_dispatcher.h"

#include "core/base/command_set.h"

namespace ark {

CommandDispatcher::CommandDispatcher(CommandDispatcher::GroupFlag groupFlag)
    : _current(sp<CommandSet>::make(groupFlag == GROUP_FLAG_EXCLUSIVE))
{
}

sp<Command> CommandDispatcher::makeCommand(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive)
{
    return sp<Command>::make(_current, onActive, onDeactive);
}

}
