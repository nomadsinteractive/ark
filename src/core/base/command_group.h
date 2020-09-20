#ifndef ARK_CORE_BASE_COMMAND_GROUP_H_
#define ARK_CORE_BASE_COMMAND_GROUP_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/command.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API CommandGroup {
public:
//  [[script::bindings::auto]]
    CommandGroup(const sp<Runnable>& onActivate, const sp<Runnable>& onDeactivate, uint32_t mask);

//  [[script::bindings::property]]
    uint32_t mask() const;
    
    const sp<Command::StateHolder>& stateHolder() const;

private:
    sp<Runnable> _on_activate;
    sp<Runnable> _on_deactivate;
    uint32_t _mask;
    
    sp<Command::StateHolder> _state_holder;
};

}

#endif
