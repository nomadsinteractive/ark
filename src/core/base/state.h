#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API State {
public:
//  [[script::bindings::auto]]
    State(sp<Runnable> onActivate = nullptr, sp<Runnable> onDeactivate = nullptr);

//  [[script::bindings::property]]
    bool active() const;

//  [[script::bindings::auto]]
    void activate();
//  [[script::bindings::auto]]
    void deactivate();

private:
    sp<Runnable> _on_activate;
    sp<Runnable> _on_deactivate;
    bool _active;
};

}
