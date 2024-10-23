#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state_action.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateActionGroup {
public:
//  [[script::bindings::auto]]
    StateActionGroup(uint32_t mask, const sp<Runnable>& onActivate = nullptr, const sp<Runnable>& onDeactivate = nullptr);

//  [[script::bindings::property]]
    uint32_t mask() const;
    
    void activate();
    void deactivate();

private:
    uint32_t _mask;
    
    std::vector<StateAction*> _commands;

    friend class StateAction;
    friend class State;
};

}
