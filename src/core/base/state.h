#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API State {
public:
//  [[script::bindings::enumeration]]
    enum LinkType {
        LINK_TYPE_SUPPORT,
        LINK_TYPE_PROPAGATE,
        LINK_TYPE_TRANSIT
    };

//  [[script::bindings::auto]]
    State(sp<Runnable> onActivate = nullptr, sp<Runnable> onDeactivate = nullptr);

//  [[script::bindings::property]]
    bool active() const;
//  [[script::bindings::property]]
    void setActive(bool active);

//  [[script::bindings::auto]]
    void activate();

//  [[script::bindings::auto]]
    void deactivate();

//  [[script::bindings::auto]]
    void createLink(State::LinkType linkType, State& nextState);

private:
    void suppress();
    void unsuppress();

    void propagate(const State& activated);
    void backPropagate(const State& deactivated);

    void doActivate();
    void doDeactivate();

    struct Link;

private:
    sp<Runnable> _on_activate;
    sp<Runnable> _on_deactivate;
    bool _active;
    bool _suppressed;

    Vector<sp<Link>> _in_links;
    Vector<sp<Link>> _out_links;
};

}
