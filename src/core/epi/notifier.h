#ifndef ARK_CORE_EPI_NOTIFIER_H_
#define ARK_CORE_EPI_NOTIFIER_H_

#include "core/base/api.h"
#include "core/collection/filtered_list.h"

namespace ark {

class ARK_API Notifier {
public:
    Notifier();

//  [[script::bindings::auto]]
    void notify() const;
//  [[script::bindings::auto]]
    sp<Observer> createObserver(bool dirty = true, const sp<Runnable>& handler = nullptr, bool oneshot = false) const;

private:
    sp<WeakRefList<Observer>> _observers;
};


}


#endif
