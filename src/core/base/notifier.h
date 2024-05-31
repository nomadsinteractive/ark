#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/list.h"

namespace ark {

class ARK_API Notifier {
public:
//  [[script::bindings::auto]]
    Notifier();
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Notifier);

//  [[script::bindings::auto]]
    void notify() const;
//  [[script::bindings::auto]]
    sp<Observer> createObserver(const sp<Runnable>& handler, bool oneshot = false) const;
//  [[script::bindings::auto]]
    sp<Boolean> createDirtyFlag(bool dirty = false) const;

private:
    class ObserverFilter {
    public:
        ObserverFilter(sp<Boolean> dirtyFlag);

        FilterAction operator() (const sp<Observer>& observer) const;

    private:
        sp<Boolean> _dirty_flag;

    };

private:
    sp<List<sp<Observer>, ObserverFilter>> _observers;
};

}
