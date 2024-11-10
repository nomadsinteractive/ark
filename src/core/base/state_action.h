#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateAction {
public:
//  [[script::bindings::auto]]
    StateAction(StateMachine& stateMachine, sp<StateActionStrand> strand, sp<Runnable> onExecute = nullptr, sp<Runnable> onActivate = nullptr, sp<Runnable> onDeactivate = nullptr);

//  [[script::bindings::property]]
    const sp<StateActionStrand>& strand() const;
//  [[script::bindings::property]]
    const sp<State>& start() const;
//  [[script::bindings::property]]
    const sp<State>& end() const;

//  [[script::bindings::property]]
    const sp<Runnable>& onExecute() const;
//  [[script::bindings::property]]
    void setOnExecute(sp<Runnable> onExecute);
//  [[script::bindings::property]]
    const sp<Runnable>& onActivate() const;
//  [[script::bindings::property]]
    void setOnActivate(sp<Runnable> onActivate);
//  [[script::bindings::property]]
    const sp<Runnable>& onDeactivate() const;
//  [[script::bindings::property]]
    void setOnDeactivate(sp<Runnable> onDeactivate);

//  [[script::bindings::property]]
    const sp<Runnable>& execute();
//  [[script::bindings::property]]
    const sp<Runnable>& activate();
//  [[script::bindings::property]]
    const sp<Runnable>& deactivate();

private:
    struct Stub {
        StateMachine& _state_machine;

        sp<StateActionStrand> _strand;

        sp<Runnable> _on_execute;
        sp<Runnable> _on_activate;
        sp<Runnable> _on_deactivate;
    };

    StateAction(sp<Stub> stub);

    struct StateActionExecute;
    struct StateActionActivate;
    struct StateActionDeactivate;

private:
    sp<Stub> _stub;

    sp<Runnable> _execute;
    sp<Runnable> _activate;
    sp<Runnable> _deactivate;

    friend class StateMachine;
    friend class StateActionStrand;
};

}
