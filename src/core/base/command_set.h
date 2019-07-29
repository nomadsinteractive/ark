#ifndef ARK_CORE_BASE_COMMAND_SET_H_
#define ARK_CORE_BASE_COMMAND_SET_H_

#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/command.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API CommandSet {
public:
    CommandSet(bool exclusive);

    void activate(const sp<Command::Stub>& toActive);
    void deactivate(const sp<Command::Stub>& toTerminate);

private:
    int32_t transfer(Command::State state, Command::State toState);

    std::set<sp<Command::Stub>> _commands;
    bool _exclusive;
};

}

#endif
