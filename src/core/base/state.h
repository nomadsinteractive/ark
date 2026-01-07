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
    sp<Boolean> active() const;
//  [[script::bindings::property]]
    sp<Boolean> suppressed() const;

    bool isActive() const;

//  [[script::bindings::auto]]
    void activate();
//  [[script::bindings::auto]]
    void deactivate();
//  [[script::bindings::auto]]
    void createLink(State::LinkType linkType, State& nextState);

private:
    void propagateSuppress(const State& from);
    void propagateUnsuppress(const State& from);

    void propagateActive(const State& from);
    void propagateDeactive(const State& from);

    void onActivate() const;
    void onDeactivate() const;

    struct Link;
    class Stub;
    class BooleanStateSuppressed;

private:
    sp<Runnable> _on_activate;
    sp<Runnable> _on_deactivate;

    Vector<sp<Link>> _in_links;
    Vector<sp<Link>> _out_links;

    sp<Stub> _stub;
};

}
