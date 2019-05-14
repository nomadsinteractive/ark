#ifndef ARK_CORE_EPI_NOTIFIER_H_
#define ARK_CORE_EPI_NOTIFIER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/list.h"

namespace ark {

class ARK_API Notifier {
public:
    Notifier();

//  [[script::bindings::auto]]
    void notify() const;
//  [[script::bindings::auto]]
    sp<Observer> createObserver(const sp<Runnable>& handler, bool oneshot = false) const;
//  [[script::bindings::auto]]
    sp<Boolean> createDirtyFlag(bool dirty = false) const;

private:
    class ObserverFilter {
    public:
        ObserverFilter(const sp<Observer>& /*item*/, sp<Boolean> dirtyFlag);

        FilterAction operator()(const sp<Observer>& item) const;

    private:
        sp<Boolean> _dirty_flag;

    };

private:
    sp<List<Observer, ObserverFilter>> _observers;
};


}


#endif
