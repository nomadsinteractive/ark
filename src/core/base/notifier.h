#ifndef ARK_CORE_BASE_NOTIFIER_H_
#define ARK_CORE_BASE_NOTIFIER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/types/weak_ptr.h"

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
        ObserverFilter(const sp<Observer>& observer, sp<Boolean> dirtyFlag);

        FilterAction operator() () const;

    private:
        WeakPtr<Observer> _observer;
        sp<Boolean> _dirty_flag;

    };

private:
    sp<List<Observer, ObserverFilter>> _observers;
};

}

#endif
