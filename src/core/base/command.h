#ifndef ARK_CORE_BASE_COMMAND_H_
#define ARK_CORE_BASE_COMMAND_H_

#include <set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Command {
public:
//  [[script::bindings::enumeration]]
    enum State {
        STATE_ACTIVATED,
        STATE_DEACTIVATED,
        STATE_PAUSED,
        STATE_COUNT
    };

public:
    Command(const sp<CommandSet>& commandSet, const sp<Runnable>& onActive, const sp<Runnable>& onDeactive);

//  [[script::bindings::property]]
    Command::State state() const;

//  [[script::bindings::auto]]
    void activate() const;
//  [[script::bindings::auto]]
    void deactivate() const;

private:
    struct Stub {
        Stub(const sp<Runnable>& onActive, const sp<Runnable>& onDeactive);

        void execute(Command::State state);

        State _state;
        sp<Runnable> _handlers[STATE_COUNT];
    };

private:
    sp<CommandSet> _command_set;
    sp<Stub> _stub;

    friend class CommandSet;
};

}

#endif
